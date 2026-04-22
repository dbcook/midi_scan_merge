#include <Arduino.h>
#include <MIDI.h>
#include <ListLib.h>
#include <MemoryFree.h>
#include "midi_const.h"
#include "debouncer.h"
#include "pin_list.h"
/*
-------------------------------------------------------------------------------
Pedalboard 32-note 8x4 matrix scanner and remapping MIDI merger for Arduino
-------------------------------------------------------------------------------

 New implementation by dbcook using a well known Arduino MIDI library.
   * Robust protocol handling all message types including SYSEX
   * Configurable number of merge channels (up to 3, so this could run a 4-manual stack)
   * Built with PlatformIO plugin for vscode
   * Parameterized to accommodate various input hardware without extensive code changes
   * Time-based debounce of both attack and release.  Runs on fast or slow CPU without changes.
   * Midi merge with configurable MIDI channel remapping for messages from each incoming port
   * Hardware wired MIDI thru is not supported
   
In the default configuration, scanned hardware notes are output to the first serial port (Serial) on MIDI channel 1.

Supported HW and pedal configurations:
   * 32-note, 4x8 diode matrix, active LOW, bottom note is C2 (MIDI note 36).  Target is assumed to
     be an ATMega 2560 with at least one and up to 3 MIDI shields.
     Rows are wired to pins 2-9 and columns are wired to pins 10-13.

Boards:
    Leonardo: 32KB flash, 2.5KB RAM, 20 digital pins of which 12 can be analog, 16MHz, built-in USB
    ATMega: 69 digital pins (xx mappable as analog, yy as PWM, zz as serial)

MIDI merge considerations:
    This code supports MIDI merge with channel-remapping, enabling a few of these Arduino scanners
    to be daisy-chained.  But this is not really a good idea from a latency perspective;
    daisy chaining of serial MIDI ports introduces 2-3 msec of delay per hop, so single notes
    from the most distant manual in a 4-manual chain could be delayed by as much as 10-12 msec.
    The last note in a big chord from the end of the chain could see 20-25 msec of latency.
*/

// MIDI DIN-5 connector pinout
//
//  PIN      MIDI IN           MIDI OUT          MIDI THRU
// ----    |-----            |------            |-----
// 1       | NC or GND       | NC or GND        | NC or GND
// 2       | SHIELD          | SHIELD           | SHIELD
// 3       | NC or +V        | NC or +V         | NC or +V
// 4       | MIDI source IN  | MIDI source OUT  | MIDI source OUT    (+5V)
// 5       | MIDI sink OUT   | MIDI sink IN     | MIDI sink IN

// The actively driven line is always the MIDI current sink (Pin 5).
// The MIDI current source is just a +5V supply through a 220 Ohm pullup resistor.
// On the MIDI IN connector, the data signal on pins 4/5 is usually hooked to an optoisolator.

// The MIDI recommended circuit connections are shown here:
// https://learn.sparkfun.com/tutorials/midi-tutorial/hardware--electronic-implementation

// Pin 2 is always the shield of a shielded 2-conductor cable.

// The DIN5 cable should be wired straight thru.  Standard cables only have 2 conductors + shield (pins 2, 4, and 5).
// A Crumar Mojo 61B lower manual needs a 4-conductor plus shield cable to support its power feed (all 5 pins wired).
// If you want to use multiple Mojo lowers, a channel-remapping MIDI merge function is required.

// CRUMAR MOJO 61B LOWER MANUAL SPECIAL MIDI CONNECTION
//
// The Crumar Mojo61B lower manual is a “passive” device (meaning it doesn’t need a separate power source, not that it’s 
// electronically passive) and is attached to the main Mojo61 via a single MIDI connector.
// It has zero onboard controls, not even output channel selection.
// Physically it's eminently stackable and even has registration holes to receive the feet of the upper manual.
// Its power must be coming on pins 1 and 3 of the connector, which are normally unused.
// The power supply voltage could be 12V or 5V depending on the circuitry of the lower manual.
// Thus the Mojo61 lower manual must use a special cable with all 5 pins wired straight thru.
// If you want to use multiples of this unit, a channel-remapping MIDI merge device is required.
// The Mojo 61B lower has been in production since 2017.


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

// how many MIDI serial ports will be merged with MIDI channel remapping
// allowable range 1-3, 1 means a single channel which implies no merging, does a channel-remapped thru only
// generally should be same as numPorts unless we consume some channel and don't merge it
#define MIDI_MERGE_CHANS 1

// flexible remapping is not implemented yet; this is simplistic
const uint8_t remapMidiChans[] = {true, true, true};
const uint8_t midiRemappedChans[] = {1, 2, 3};

// Pointers to pre-existing HardwareSerial objects (board dependent).  Set these to be the desired set of ports to be serviced by the MIDI RX/merge engine.
HardwareSerial *serialPorts[] = {
      &Serial3
    // , &Serial1
    // , &Serial2
};
const uint8_t numPorts = sizeof(serialPorts) / sizeof(serialPorts[0]);

// implemented functions are midiMergeThru, midiMerge2, midiMerge3
#if MIDI_MERGE_CHANS == 1
#define MIDI_MERGE_FUNC midiMergeThru
#elif MIDI_MERGE_CHANS == 2
#define MIDI_MERGE_FUNC midiMerge2
#elif MIDI_MERGE_CHANS == 3
#define MIDI_MERGE_FUNC midiMerge3
#else
#endif

// Midi output channel for scanned notes
// *** TODO: persist in NVRAM and set/query via sysex msg
#define MERGE_OUTPUT_PORT_MAPPED_CHAN 0

// Set up the MIDI library - compile time macros
// the operation of the macros prevents turning the midi0, midi1, etc. into an indexed array
MIDI_CREATE_INSTANCE(HardwareSerial, *serialPorts[0], midi0);

#if MIDI_MERGE_CHANS > 1
MIDI_CREATE_INSTANCE(HardwareSerial, *serialPorts[1], midi1);
#endif

#if MIDI_MERGE_CHANS > 2
MIDI_CREATE_INSTANCE(HardwareSerial, *serialPorts[2], midi2);
#endif

#define MERGE_OUTPUT_PORT midi0

class DiodeMatrixBase {
    protected:
        int colBasePin;
        int numColPins;
        int rowBasePin;
        int numRowPins;
        bool activeLow;
        int midiOutChan;

        t_midiInterfaceHWSerialPtr midiInterface;

        virtual void configureHw() = 0;
        virtual uint8_t getMidiNoteNumFromPins(int baseMidiNote, int colPin, int rowPin) {
            return baseMidiNote + (colPin - colBasePin) * numRowPins + (rowPin - rowBasePin);
        }

    public:
        DiodeMatrixBase(
            t_midiInterfaceHWSerialPtr midi,
            int midiOutChan,
            int colBase, int numCols, int rowBase, int numRows,
            bool activeLow = true)
        {
            this->midiInterface = midi;
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
        DiodeMatrixPedalboard_4x8(t_midiInterfaceHWSerialPtr pMidi, int midiOutChan) : DiodeMatrixBase(pMidi, midiOutChan, 16, 8, 24, 16, true) {
            int note = 0;
            for (int colPin = this->colBasePin; colPin < this->colBasePin + this->numColPins; colPin++) {
                for (int rowPin = this->rowBasePin; rowPin < this->rowBasePin + this->numRowPins; rowPin++) {
                    debouncers[note++] = new DebouncerMidiNoteSingleContact(debounceMsec, pMidi, getMidiNoteNumFromPins(baseMidiNote, colPin, rowPin), midiOutChan);
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


DiodeMatrixPedalboard_4x8 dmatrix(&midi0, MERGE_OUTPUT_PORT_MAPPED_CHAN);

unsigned long lastMillis = 0;
uint8_t ledState = LOW;
unsigned long loopCount = 0;

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


