#pragma once
#include <Arduino.h>
#include <MIDI.h>

#include "glob_gen.h"
#include "debug.h"
#include "midi_const.h"
#include "data.h"

// *** think about how to debounce in velocity mode where we have to consider two related signals
//     need info from two different matrix objects, so need class VelocityMatrixPair or similar

// *** TODO: add debouncers for
//       debouncing dual contact velocity sensitive note-on/off sensors and triple-contact aftertouch keyboards
//       sending CC messages based on an analog encoder input (requires refactoring DebouncerBase a bit)
//       SysRealtime messages based on a contact closure 

// Base class for bidirectional (attack and release) switch debounder
// Derived classes can support debouncers for MIDI notes, knobs and analog pedals (CC messages), and even non-MIDI interfaces

class DebouncerBase {
    protected:
        uint8_t attackDebounceMsec = 20;              // stabilization time for the input signal inactive->active
        uint8_t releaseDebounceMsec = 5;              // debounce time for active->inactive transition

        DebouncerBase() {}

    public:

        virtual void reset() {
            attackDebounceMsec = 20;
            releaseDebounceMsec = 5;
        }

};



// Debouncer for analog inputs (pot, rotary encoder, etc.) that generate CC channel messages
class DebouncerMidiCCAnalog : public DebouncerBase {
    protected:
        int filteredCtrlValue = 0;          // running filtered control value

        // Lowpass filter params and tracking state
        int fuzzTolerance = 0;              // how much sample-to-sample high frequency jitter (in analog input space) is allowed while still considered stable
        int stableTrendThresh = 0;          // how much stable drift is allowed (with successive samples not violating fuzzTolerance) before we send a new CC value
        unsigned long stableTStamp = 0;     // Tms at which the input became stable
        bool inputIsStable = false;         // assume initially unstable so that messages will establish control values at startup
        midi::DataByte controlNum = 0;
        midi::Channel midiOutChan = 0;

    public:
        DebouncerMidiCCAnalog() : DebouncerBase() {}

        void setFuzzAndTrend(int fuzzTolerance, int stableTrendThresh) {
            this->fuzzTolerance = fuzzTolerance;
            this->stableTrendThresh = stableTrendThresh;
        }

        midi::DataByte scaleInput(int inputVal) {
            return inputVal >> 8;           // *** FIXME ***
        }

        void setControlValue();

        void deactivateControl() {}

        void reset() {
            DebouncerBase::reset();
        }

        // run a low pass filter on the input samples and emit CC messages when
        //    transitions from unstable to stable; or
        //    stable but LPF output value has changed by more than stableTrendThresh from previously emitted value
        void stateSample(int rawValue) {

        }
};


// Debouncer for bo†h matrix and parallel scanners that generate note-on and note-off messages

// As they currently exist, these debouncers take 14 bytes each so an 8x8 matrix takes up 896 bytes of RAM.
// Various fields that are common to the entire pinBlock have been kept as member vars since args to stateSample() cost a
// lot of execution time.
class DebouncerMidiNoteSingleContact : public DebouncerBase {
    protected:
        const uint8_t midiDefaultVelocity = MIDI_VELOCITY_MAX;
    
        unsigned long activeTStamp = 0;         // time at which the input was first seen in the active state after being inactive
        unsigned long inactiveTStamp = 0;       // time at which the input was first seen in the inactive state after being active
        bool inputIsActive = false;             // true if the last input sample was active
        bool ctrlIsOn = false;                  // true if the debounced control (note, switch, piston, ...) is currently active
        midi::DataByte noteNum = 0;             // note number to be emitted
        midi::Channel midiOutChan = 0;          // output MIDI channel number

    public:

        DebouncerMidiNoteSingleContact() {}

        void reset() {
            DebouncerBase::reset();
            activeTStamp = 0;
            inactiveTStamp = 0;
            inputIsActive = false;
            ctrlIsOn = false;
        }

        void setNoteAndChan( midi::DataByte note, midi::Channel outChan) {
            noteNum = note;
            midiOutChan = outChan;
        }

        void setDebounceTimes(uint8_t attackDebounce, uint8_t releaseDebounce) {
            attackDebounceMsec = attackDebounce;
            releaseDebounceMsec = releaseDebounce;
        }

        void stateSample(bool sampleActive, midi::DataByte noteNum, midi::Channel midiOutChan);
        void stateSampleActive();
        void stateSampleInactive();

        void activateControl();
        void deactivateControl();

};

