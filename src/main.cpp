#include <Arduino.h>
#include <MIDI.h>
#include <ListLib.h>
#include <MemoryFree.h>

#define GEN_GLOBALS
#include "glob_gen.h"
#include "fastread.h"
#include "debug.h"
#include "data.h"
#include "midi_const.h"
#include "debouncer.h"
#include "pin_list.h"

/*
-------------------------------------------------------------------------------
MIDI input scanner and remapping MIDI merger for Arduino Mega 2560 and others
-------------------------------------------------------------------------------

New implementation of a MIDI scanner / merger by dbcook using the well known 
47Effects Arduino MIDI library.

Supports any combination of diode matrix and parallel input arrays up to a total of 65 inputs.
With four 8x8 diode matrix arrays you get a max scanning capacity of 256 controls or notes.

Arduino Boards Info:
    ATMega: 8KB RAM, 253,952 B flash, 69 digital pins (16 mappable as analog, 8 pins as 4 hardware serial ports), 16 MHz
    Leonardo: 32KB flash, 2.5KB RAM, 20 digital pins of which 12 can be analog, 16MHz, class-compliant USB host

The initial implementation uses the simplistic Arduino digitalRead() to scan one input
bit at a time.  This however includes various safety checks and mapping, and costs ~100 cycles
per input line (said to be ~3 usec on an Uno whereas a direct op is ~125 nanosec).  It will be drastically
faster to use the digitalWriteFast library (which also includes read functions)
or even to exploit the PINA, PINB PINC PIND registers directly.

I can imagine an implementation where we set a select pin using digitalWriteFast (hehe settling time
possibly needed after!) and then snapshot all of the inputs at once by reading the registers.
Then with compile-time operations we can quickly create a byte array in memory where any nonzero
value means the input was active. This modus operandi is exactly what the digitalWriteFast library is doing.

In order for this to work the pin number has to be a compile time constant.  To use this with our
flexible pin block definition scheme we'll need a function that resolves pin number ranges to
specific digitalWriteFast macros.  That will take some code and instruction cycles but we have tons of flash.

An even faster way will be to write code for fixed layouts of common blocks (8x8, 4x8) using fixed pin
assignments for the select and read lines.
*/

// scan component speed test routines

void test_fastread() {
    for (int i = 2; i < 69; i++) {
        fastread(i);
    }
}

void test_diodeMatrix_8x8() {
    int buf[8];
    for (int colPin = 20; colPin < 28; colPin++) {
        fastwrite(colPin, LOW);
        int indx = 0;
        for (int readPin = 28; readPin < 36; readPin++) {
            buf[indx++] = fastread(readPin);
        }
        fastwrite(colPin, HIGH);
    }
}


// new scan block processor based on flash PinBlock defs
void scanPinBlocks() {
    for (int i = 0; i < nPinBlocks; i++) {
        // copying the PinBlock struct into RAM is almost twice as slow as reading directly from flash
        const PinBlock_t * pb = &(gPinBlocks[i]);
        midi::DataByte noteNum = pb->baseMidiNoteNum;
        int dbIndx = gDebouncerBases[i];

        // These help confirm that the PinBlock is being read and processed correctly
#if 0
        Console_print("cbase "); Console_println(pb->selectBasePin);
        Console_print("clim "); Console_println(pb->selectBasePin + pb->numSelectPins);
        Console_print("rbase "); Console_println(pb->readBasePin);
        Console_print("rlim "); Console_println(pb->readBasePin + pb->numReadPins);
#endif
        // process the pin block
        if (pb->useSelect) {
            // diode matrix - read pins loop inside of select pins loop
            midi::DataByte noteLim = pb->baseMidiNoteNum + pb->numCtrls;        // in case matrix has a non-full select row
            int clim = pb->selectBasePin + pb->numSelectPins;
            int rlim = pb->readBasePin + pb->numReadPins;
            for (int selPin = pb->selectBasePin; selPin < clim; selPin++) {
                digitalWrite(selPin, pb->activeLow ? LOW : HIGH);  // activate the select pin
                //fastwrite(colPin, pb->activeLow ? LOW : HIGH);

                // scan the read pins
                for (int readPin = pb->readBasePin; (readPin < rlim) && (noteNum < noteLim); readPin++) {
                    //int inp = digitalRead(rowPin);
                    int inp = fastread(readPin);

                    //gDebouncers[dbIndx++].stateSampleDummy(pb->activeLow ? !inp : inp, noteNum++, pb->midiOutChan); // logs notes and chans sequence
                    //gDebouncers[dbIndx++].stateSample(true, noteNum++, pb->midiOutChan);  // causes initial burst of noteOn for all notes
                    //gDebouncers[dbIndx++].stateSample(false, noteNum++, pb->midiOutChan);  // make sure it never sees anything

                    gDebouncers[dbIndx++].stateSample(pb->activeLow ? !inp : inp, noteNum++, pb->midiOutChan);
                }
                //digitalWrite(colPin, pb->activeLow ? HIGH : LOW);  // deactivate the select pin
                fastwrite(selPin, pb->activeLow ? HIGH : LOW);  // deactivate the select pin
            }
        }
        else {
            // parallel non-matrix inputs - single loop on read pins
            int rlim = pb->readBasePin + pb->numReadPins;
            for (int readPin = pb->readBasePin; readPin < rlim; readPin++) {
                int inp = digitalRead(readPin);
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

    //test_fastread();
    //test_diodeMatrix_8x8();
    scanPinBlocks();
}


