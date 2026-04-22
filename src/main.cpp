#include <Arduino.h>
#include <MIDI.h>
#include <ListLib.h>
#include <MemoryFree.h>

#define GEN_GLOBALS
#include "glob_gen.h"
#include "data.h"
#include "midi_const.h"
#include "debouncer.h"
#include "pin_list.h"

/*
-------------------------------------------------------------------------------
Pedalboard 32-note 8x4 matrix scanner and remapping MIDI merger for Arduino
-------------------------------------------------------------------------------

 New implementation by dbcook using a well known Arduino MIDI library.
   
In the default configuration, scanned hardware notes are output to the first serial port (Serial) on MIDI channel 1.

Supported HW and pedal configurations:
   * 32-note, 4x8 diode matrix, active LOW, bottom note is C2 (MIDI note 36).  Target is assumed to
     be an ATMega 2560 with at least one and up to 3 MIDI shields.
     Rows are wired to pins 2-9 and columns are wired to pins 10-13.

Boards:
    Leonardo: 32KB flash, 2.5KB RAM, 20 digital pins of which 12 can be analog, 16MHz, built-in USB
    ATMega: 69 digital pins (xx mappable as analog, yy as PWM, zz as serial)

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

// Debug console system
// All console code vanishes when USE_DEBUG_PRINT is false
// If USE_DEBUG_PRINT is true, the MIDI shield must not use Serial but must use Serial1 - Serial3
#define USE_DEBUG_PRINT true

#if USE_DEBUG_PRINT
const unsigned long consoleBaudRate = 115200;
HardwareSerial *Console = &Serial;

#define Console_print(...) Console->print(__VA_ARGS__)
#define Console_println(...) Console->println(__VA_ARGS__)
#define Console_flush Console->flush()
#else
#define Console_print(...)
#define Console_println(...)
#define Console_flush(...)
#endif

// Hardware setup for Mega 2560
#define MEGA2560_LED_PIN 13
#define LED_PIN MEGA2560_LED_PIN


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

// Sizing for the input keyboard/pedalboard compass
#define BASE_NOTENUM PEDAL_LOW_NOTENUM
#define NUM_NOTES PEDAL32_MAX_NOTES

// MIDI standard interface serial rate is 31250 baud, which is stupid slow
// NOTE ON are 3 byte messages so max ~1KHz msg rate, 2 byte msgs if using
// "running status" would allow 1.5KHz message rate; however running status is not universally understood.
// If you put down a big chord with 10-15 notes at once it will take over 10msec of wire time alone to transmit the chord
// *PLUS* processing time at all merge points and the receiving end.  That is NOT going to be inaudible delay; huge case for
// doing MIDI over USB if the jitter there is decently controlled.


// ----------------------------------------------------------------------------
// MIDI message types and masks not used in this code but helpful for reference
//  *** These are likely defined in the 47Effects MIDI library, remove when found ***

// Status byte values.  Except as indicated, all are followed by 2 data bytes.
#define MIDI_STATUS_MASK 0x80
#define MIDI_CHANNEL_MASK 0x0F

// All system messages have no channel number; instead having extended opcodes
// system common messages
#define MIDI_SYSTEM 0xF0                // has subclasses based on low nibble
#define MIDI_SYSEX 0xF0                 // variable length, terminated by MIDI_EOX, 7 bit encoded
#define MIDI_EOX 0xF7                   // end of SYSEX marker
#define MIDI_SYS_MTC 0xF1               // MIDI Time Code, 1 byte
#define MIDI_SYS_SONGPOS 0xF2           // Song Position Pointer, 2 bytes
#define MIDI_SYS_SONGSEL 0xF3           // Song select, 1 byte
#define MIDI_SYS_CABLESEL 0xF5          // Cable select, 1 byte
#define MIDI_SYS_TUNEREQ 0xF6           // Tune request, 0 bytes

// system realtime, all with 0 data bytes
#define MIDI_SYS_TIMING 0xF8            // Timing clock, sent at 24ppqn when clock is running
#define MIDI_SYS_START 0xFA             // Start - clock starts
#define MIDI_SYS_CONTINUE 0xFB          // Continue - clock continues after stop
#define MIDI_SYS_STOP 0xFC              // Stop - when clock stops
#define MIDI_SYS_ACTIVESENSING 0xFE     // Active sensing, sent by some devices when idle to indicate connection is alive
#define MIDI_SYS_RESET 0xFF             // System reset - resets all devices to power-up state = panic button


#define MIDI_NOTE_ON 0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_POLY_PRESSURE 0xA0
#define MIDI_CONTROL_CHANGE 0xB0
#define MIDI_PROGRAM_CHANGE 0xC0        // 1 data byte
#define MIDI_CHANNEL_PRESSURE 0xD0      // 1 data byte
#define MIDI_PITCH_BEND 0xE0

class DiodeMatrixBase {
    protected:
        int colBasePin;
        int numColPins;
        int rowBasePin;
        int numRowPins;
        bool activeLow;
        int midiOutChan;

        virtual void configureHw() = 0;
        virtual uint8_t getMidiNoteNumFromPins(int baseMidiNote, int colPin, int rowPin) {
            return baseMidiNote + (colPin - colBasePin) * numRowPins + (rowPin - rowBasePin);
        }

    public:
        DiodeMatrixBase(
            int midiOutChan,
            int colBase, int numCols, int rowBase, int numRows,
            bool activeLow = true)
        {
            this->midiOutChan = midiOutChan;
            this->colBasePin = colBase;
            this->numColPins = numCols;
            this->rowBasePin = rowBase;
            this->numRowPins = numRows;
            this->activeLow = activeLow;

        }
        int getColBasePin() { return this->colBasePin; }
        int getNumColPins() { return this->numColPins; }
        int getRowBasePin() { return this->rowBasePin; }
        int getNumRowPins() { return this->numRowPins; }
        bool getActiveLow() { return this->activeLow; }
        
        void setColPinActive(int pin) {
            digitalWrite(pin, this->activeLow ? LOW : HIGH);
        }
        void setColPinInactive(int pin) {
            digitalWrite(pin, this->activeLow ? HIGH : LOW);
        }
        bool translateInputSample(int rawInput) {
            if (this->activeLow)
                return rawInput == LOW ? true : false;
            else
                return rawInput == HIGH ? true : false;
        }

};


// Diode matrix scanning for a 4 cols x 8 rows matrix (usually for a pedalboard) where:
//   column pins on the Arduino are contiguous
//   row pins on the Arduino are contiguous
//   the "column" pins are written as selectors, the row pins are read as the input
//   MIDI note numbers start at col 0 row 0, proceeding row-wise as [0,0] [0,1] etc.
class DiodeMatrixPedalboard_4x8 : public DiodeMatrixBase {
    protected:
        DebouncerMidiNoteSingleContact* debouncers[MIDI_MAX_NOTES];
        long debounceMsec = 20;
        const int baseMidiNote = PEDAL_LOW_NOTENUM;

        void configureHw() {
            uint8_t rowPinMode = activeLow ? INPUT_PULLUP : INPUT;
            for (int i = colBasePin; i < colBasePin + numColPins; i++ ) {
                pinMode(i, OUTPUT);
                digitalWrite(i, HIGH);  // col select same for active low and active high
            }
            for (int j = rowBasePin; j < rowBasePin + numRowPins; j++ ) {
                pinMode(j, rowPinMode);
            }
        }

    public:
        // crashes if we allocate 8 cols x 24 rows, 20 works ok with freemem 4978
        DiodeMatrixPedalboard_4x8(int midiOutChan) : DiodeMatrixBase(midiOutChan, 16, 8, 24, 8, true) {
            int note = 0;
            for (int colPin = this->colBasePin; colPin < this->colBasePin + this->numColPins; colPin++) {
                for (int rowPin = this->rowBasePin; rowPin < this->rowBasePin + this->numRowPins; rowPin++) {
                    debouncers[note] = new DebouncerMidiNoteSingleContact(debounceMsec, getMidiNoteNumFromPins(baseMidiNote, colPin, rowPin), midiOutChan);
                    note++;
                }
            }
            configureHw();
        }

        // [column, row] scanner for single-contact keys
        void scan( ) {
            for (int colPin = getColBasePin(); colPin < getColBasePin() + getNumColPins(); colPin++)
            {
                setColPinActive(colPin);

                for (int rowPin = getRowBasePin(); rowPin < getNumRowPins(); rowPin++)
                {
                    uint8_t noteNumber = getMidiNoteNumFromPins(baseMidiNote, colPin, rowPin);
                    DebouncerMidiNoteSingleContact* debouncer = this->debouncers[noteNumber];
                    bool inputActive  = translateInputSample(digitalRead(rowPin));
                    debouncer->stateSample(inputActive);
                }
                setColPinInactive(colPin);
            }
        }
};




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
// WARNING - the basic merge/thru functions below only support 3-byte MIDI messages.
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


DiodeMatrixPedalboard_4x8 dmatrix(MERGE_OUTPUT_PORT_MAPPED_CHAN);

//const MatrixEnt_Single_Static_t foo[] = {{true, 10, 20, 20, 4}};

void setup() 
{
    pinMode(LED_PIN, OUTPUT);

#if USE_DEBUG_PRINT
    Console->begin(consoleBaudRate);
#endif
    
    // MIDI related hw is configured upon instantiation of the diode matrix object

    startMidi();
    // delay(STARTUP_DELAY_MS);
}

// statics for the main loop
unsigned long lastMillis = 0;
unsigned long loopCount = 0;

void loop() 
{
    // loop rate tracking results
    //   MIDI merge checking time is negligible, < 10 microsec per merged port
    //   8x8 diode matrix takes about 100 usec each => can do 4 of them at 2 KHz
    //    raw rate with nothing but rate tracking:             165.7 KHz +/- 0.1 KHz
    //    calling MIDI_MERGE_FUNC with num merge channels = 1: 109.0 KHz
    //    calling 4x8 matrix scanner                         :  17.5 KHz
    //    calling 8x8 matrix scanner                             9.64 KHz
    loopCount++;
    unsigned long curMillis = millis();
    if (curMillis - lastMillis > 1000) {
        lastMillis = curMillis;
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        Console_print("rate: "); Console_println(loopCount);
        Console_print("memfree: "); Console_println(freeMemory());
        loopCount = 0;
    }

    MIDI_MERGE_FUNC();

    dmatrix.scan();
}


