#include <Arduino.h>
#include <MIDI.h>
#include <ListLib.h>
#include <MemoryFree.h>

#define GEN_GLOBALS
#include "glob_gen.h"
#include "debug.h"
#include "data.h"
#include "midi_const.h"
#include "debouncer.h"
#include "pin_list.h"

/*
-------------------------------------------------------------------------------
MIDI input scanner and remapping MIDI merger for Arduino Mega 2560 and others
-------------------------------------------------------------------------------

New implementation of a MIDI scanner / merger by dbcook using a well known Arduino MIDI library.

Supports any combination of diode matrix and parallel input arrays up to a total of 65 inputs.
With four 8x8 diode matrix arrays you get a max scanning capacity of 256 controls or notes.

Arduino Board Info:
    ATMega: 8KB RAM, 253,952 B flash, 69 digital pins (16 mappable as analog, 8 pins as 4 hardware serial ports), 16 MHz
    Leonardo: 32KB flash, 2.5KB RAM, 20 digital pins of which 12 can be analog, 16MHz, class-compliant USB host

*/



// MIDI NOTE NUMBERING
//
//      note number of low C on a full A?GO pedalboard starts at 36 with 32 notes following sequentially
//      MIDI note numbers are zero-based, start at C(-1) == 0 and go thru G9 == 127 (when A4 == A440 concert pitch and C4 == piano middle C)
//      Middle C == MIDI note 60 is the nailed-down reference.  Octave numbering is a free-for-all.
//          Some references have C0 == 0, numerous mfrs call C3 == middle C
//          I adopt the convention that middle C == C4
//      61-note organ keyboard compass is C2 to C7 (MIDI notes 36-96)
//      73-note keyboard adds octave above, C2 to C8 (MIDI notes 36-108)
//      88-note keyboard add octave+3 downward, A0 to C8 (Midi notes 21-108)
//      27-note pedalboard compass is C2 to D4 (Midi notes 36-62)
//      30-note pdealboard compass is C2 to F4 (Midi notes 36-65)
//      32-note pedalboard compass is C2 to G4 (Midi notes 36-67)
//      *** do note numbers shift when transposing functions are active?


// ----------------------------------------------------------------------------
// Build config
// ----------------------------------------------------------------------------

// Hardware setup for Mega 2560


// *** not sure how much startup delay is justified - what are we waiting for?  Power supply stability?  Scope time...
// The inputs in a diode matrix have no noticeable intrinsic setup time, nor serial ports.
#define STARTUP_DELAY_MS 500

// dbc: generalized code follows.  Ultra short pedalboards not considered.
#define MIDI_NOTENUM_A0     21         // low note on 88-key keyboard
#define MIDI_NOTENUM_C2     36         // low note on 61 and 73 key keyboards

#define PEDAL27_MAX_NOTES   27
#define PEDAL30_MAX_NOTES   30
#define PEDAL32_MAX_NOTES   32
// All normal compass pedalboards start at C2
#define PEDAL_LOW_NOTENUM MIDI_NOTENUM_C2

#define KEYBOARD61_LOW_NOTENUM MIDI_NOTENUM_C2
#define KEYBOARD73_LOW_NOTENUM MIDI_NOTENUM_C2
#define KEYBOARD88_LOW_NOTENUM MIDI_NOTENUM_A0
#define KEYBOARD61_MAX_NOTES 61
#define KEYBOARD73_MAX_NOTES 73
#define KEYBOARD88_MAX_NOTES 88

// MIDI standard interface serial rate is 31250 baud, which is stupid slow
// NOTE ON are 3 byte messages so max ~1KHz msg rate, 2 byte msgs if using
// "running status" would allow 1.5KHz message rate; however running status is not universally understood.
// If you put down a big chord with 10-15 notes at once it will take over 10msec of wire time alone to transmit the chord
// *PLUS* processing time at all merge points and the receiving end.  That is NOT going to be inaudible delay; huge case for
// doing MIDI over USB if the jitter there is decently controlled.



// new scan block processor based on flash PinBlock defs
void scanPinBlocks() {
    for (int i = 0; i < nPinBlocks; i++) {
        // copying the PinBlock struct into RAM is almost twice as slow as reading directly from flash
        const PinBlock_t * pb = &(gPinBlocks[i]);
        midi::DataByte noteNum = pb->baseMidiNoteNum;
        int dbIndx = gDebouncerBases[i];

        // These help confirm that the PinBlock is being read and processed correctly
        // Console_print("cbase "); Console_println(pb->selectBasePin);
        // Console_print("clim "); Console_println(pb->selectBasePin + pb->numSelectPins);
        // Console_print("rbase "); Console_println(pb->readBasePin);
        // Console_print("rlim "); Console_println(pb->readBasePin + pb->numReadPins);

        // process the pin block
        if (pb->useSelect) {
            // diode matrix - read pins loop inside of select pins loop
            midi::DataByte noteLim = pb->baseMidiNoteNum + pb->numCtrls;        // in case matrix has a non-full select row
            int clim = pb->selectBasePin + pb->numSelectPins;
            int rlim = pb->readBasePin + pb->numReadPins;
            for (int colPin = pb->selectBasePin; colPin < clim; colPin++) {
                digitalWrite(colPin, pb->activeLow ? LOW : HIGH);  // activate the select pin

                // scan the read pins
                for (int rowPin = pb->readBasePin; (rowPin < rlim) && (noteNum < noteLim); rowPin++) {
                    int inp = digitalRead(rowPin);

                    //gDebouncers[dbIndx++].stateSampleDummy(pb->activeLow ? !inp : inp, noteNum++, pb->midiOutChan); // logs notes and chans sequence
                    //gDebouncers[dbIndx++].stateSample(true, noteNum++, pb->midiOutChan);  // causes initial burst of noteOn for all notes
                    //gDebouncers[dbIndx++].stateSample(false, noteNum++, pb->midiOutChan);  // make sure it never sees anything

                    gDebouncers[dbIndx++].stateSample(pb->activeLow ? !inp : inp, noteNum++, pb->midiOutChan);
                }
                digitalWrite(colPin, pb->activeLow ? HIGH : LOW);  // deactivate the select pin
            }
        }
        else {
            // parallel non-matrix inputs - single loop on read pins
            int rlim = pb->readBasePin + pb->numReadPins;
            for (int rowPin = pb->readBasePin; rowPin < rlim; rowPin++) {
                int inp = digitalRead(rowPin);
                gDebouncers[dbIndx++].stateSample(pb->activeLow ? !inp : inp, noteNum++, pb->midiOutChan);
            }
        }

    }
}



// If we want to do MIDI channel remapping we have to filter the library's normal THRU handling.
//
// If you start the port with OMNI channels, everything will be sent onward, *unless* you set the thru filter
// mode to "other channels" via setThruFilterMode(DifferentChannel).
//  This *should* stop automatic THRU processing of channelized messages (note on/off etc.)
// In all cases, non-channelized messages are sent onwards unless THRU is turned off entirely
// for the port.
//
// In order to remap channelized messages AND preserve THRU handling of non-channelized messages, 
// the way to do it is by implementing a message handling callback that will do the remapping of
// channelized messages.
//
// WARNING - the basic merge/thru functions below only support 3-byte MIDI channel messages.
void startMidi()
{
    midi0.turnThruOff();
    midi0.begin(MIDI_CHANNEL_OMNI);

#if MIDI_MERGE_CHANS > 1
    midi1.turnThruOff();
    midi1.begin(MIDI_CHANNEL_OMNI);
#endif

#if MIDI_MERGE_CHANS > 2
    midi2.turnThruOff();
    midi2.begin(MIDI_CHANNEL_OMNI);
#endif
}


// does a thru but with optional channel remapping
void midiMergeThru() {
    if (midi0.read()) {
        Console_println("midi0 msg rx");
        uint8_t outChan = remapMidiChans[0] ? midiRemappedChans[0] : midi0.getChannel();
        midi0.send(
            midi0.getType(),
            midi0.getData1(),
            midi0.getData2(),
            outChan
        );
    }
}

#if MIDI_MERGE_CHANS >= 2
// 2 channel MIDI merge - forwards one additional channels
void midiMerge2()
{
    midiMergeThru();
    if (midi1.read()) {
        uint8_t outChan = remapMidiChans[1] ? midiRemappedChans[1] : midi1.getChannel();
        midi0.send(
            midi1.getType(),
            midi1.getData1(),
            midi1.getData2(),
            outChan
        );
    }
}
#endif

#if MIDI_MERGE_CHANS == 3
// 3 channel MIDI merge - forwards two additional channels
void midiMerge3()
{
    midiMerge2();
    if (midi2.read()) {
        uint8_t outChan = remapMidiChans[2] ? midiRemappedChans[2] : midi2.getChannel();
        midi0.send(
            midi2.getType(),
            midi2.getData1(),
            midi2.getData2(),
            outChan
        );
    }
}
#endif


void configurePins() {
    for (int i = 0; i < nPinBlocks; i++) {
        const PinBlock_t * pb = &gPinBlocks[i];

        if (pb->useSelect) {
            for (uint8_t selPin = pb->selectBasePin; selPin < pb->selectBasePin + pb->numSelectPins; selPin++) {
                pinMode(selPin, OUTPUT);
                digitalWrite(selPin, pb->activeLow ? HIGH : LOW);
            }
        }
        for (uint8_t readPin = pb->readBasePin; readPin < pb->readBasePin + pb->numReadPins; readPin++) {
            pinMode(readPin, pb->activeLow ? INPUT_PULLUP : INPUT);
        }
    }

}


void setup() 
{
    pinMode(LED_BUILTIN, OUTPUT);

#if USE_DEBUG_PRINT
    Console->begin(consoleBaudRate);
#endif

    initDebouncers();
    initDebouncerBases();

    configurePins();
    
    startMidi();
}

// statics for the main loop
unsigned long lastMillis = 0;
unsigned long loopCount = 0;

void loop() 
{
    // loop rate tracking
    //   MIDI merge checking time is negligible, < 10 microsec per merged port
    //   For reasons yet to be determined, diode matrix scanning time is a little nonlinear vs matrix size.
    //    raw rate with nothing but rate tracking:             165.7 KHz +/- 0.1 KHz
    //    only MIDI_MERGE_FUNC with num merge channels = 1.    117.0 KHz
    //    single 8x4 matrix block                                2.64 KHz (379 usec = 11.8 usec per input)
    //    single 8x8 matrix block                                1.56 KHz (pins 16-31 = 10.0 usec per input)  1.44 KHz (pins 32-48 = 10.8 usec per input)
    //    two 8x8 matrix blocks                                  0.641 KHz (1560 usec = 12.2 usec per input )
    //    single block of 32 parallels                           3.48 KHz (287 usec = 8.97 usec per input)
    loopCount++;
    unsigned long curMillis = millis();
    if (curMillis - lastMillis > 1000) {
        lastMillis = curMillis;
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        Console_print("rate: "); Console_println(loopCount);
        Console_print("memfree: "); Console_println(freeMemory());
        loopCount = 0;
    }

    MIDI_MERGE_FUNC();

    scanPinBlocks();
}


