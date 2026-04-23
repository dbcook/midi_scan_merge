#pragma once
#include <Arduino.h>
#include <MIDI.h>

#include "glob_gen.h"
#include "debug.h"
#include "midi_const.h"
#include "data.h"

// *** think about how to debounce in velocity mode where we have to consider two related signals
//     need info from two different matrix objects, so need class VelocityMatrixPair

// *** TODO: add debouncers for
//       debouncing dual contact note-on/off sensors
//       sending CC messages based on an analog encoder input (requires refactoring DebouncerBase a bit)
//       SysRealtime messages based on a contact closure 

// Base class for bidirectional (attack and release) switch debounder
// Derived classes can support debouncers for MIDI notes, buttons/switches/pistons (CC messages), and even non-MIDI interfaces

class DebouncerBase {
    protected:
        uint8_t debounceMsec = 20;              // stabilization time for the input signal

        DebouncerBase() {}

    public:

        virtual void reset() {
            debounceMsec = 20;
        }

        // Abstraction issues for stateSample - takes different args depending on derived class
        //virtual void stateSample(bool sampleActive) = 0;

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

    public:
        DebouncerMidiCCAnalog() : DebouncerBase() {}

        void setFuzzAndTrend(int fuzzTolerance, int stableTrendThresh) {
            this->fuzzTolerance = fuzzTolerance;
            this->stableTrendThresh = stableTrendThresh;
        }

        midi::DataByte scaleInput(int inputVal) {
            return inputVal >> 8;           // *** FIXME ***
        }

        void setControlValue(midi::DataByte controlNum, midi::Channel midiOutChan);

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

// As they currently exist after eliminating the MIDI interface pointer, these debouncers take 14 bytes
// each so an 8x8 matrix takes up 896 bytes of RAM.
// It looks like we could recover 2 more bytes though by eliminating the note number and using the global
// MIDI output channel.
class DebouncerMidiNoteSingleContact : public DebouncerBase {
    protected:
        const uint8_t midiDefaultVelocity = MIDI_VELOCITY_MAX;
    
        unsigned long activeTStamp = 0;         // time at which the input was first seen in the active state after being inactive
        unsigned long inactiveTStamp = 0;       // time at which the input was first seen in the inactive state after being active
        bool inputIsActive = false;             // true if the last input sample was active
        bool ctrlIsOn = false;                  // true if the debounced control (note, switch, piston, ...) is currently active

    public:

        DebouncerMidiNoteSingleContact() {
        }

        void activateControl(midi::DataByte noteNum, midi::Channel midiOutChan);

        void deactivateControl(midi::DataByte noteNum, midi::Channel midiOutChan);

        void reset() {
            DebouncerBase::reset();
            activeTStamp = 0;
            inactiveTStamp = 0;
            inputIsActive = false;
            ctrlIsOn = false;
        }

        // call this instead of the regular stateSample to verify the scanning sequence is correct
        void stateSampleDummy(int sampleActive, midi::DataByte noteNum, midi::Channel midiOutChan) {
            Console_print("note "); Console_println(noteNum);
            Console_print("chan "); Console_println(midiOutChan);
            Console_print("sample "); Console_println(sampleActive);
        }

        void stateSample(int sampleActive, midi::DataByte noteNum, midi::Channel midiOutChan) {
            if (!sampleActive) {
                if (!this->ctrlIsOn) return;    // sample is off, note is off, bail
                // sample is off, but note is on - are we transitioning to release debounce state?
                if (this->inputIsActive) {
                    inactiveTStamp = millis();  // record time of transition
                    this->inputIsActive = false;
                }
                // now we know note is on but we are in release debounce state
                // see if debounce interval is satisfied
                if ((millis() - this->inactiveTStamp) >= this->debounceMsec) {
                    this->ctrlIsOn = false;
                    Console_print("note off "); Console_println(noteNum);
                    this->deactivateControl(noteNum, midiOutChan);
                }
            }
            else {
                if (this->ctrlIsOn) return;     // note is already on, we're done
                // are we transitioning to input-active state (starting debounce)
                if (!this->inputIsActive) {
                    activeTStamp = millis();    // record time of transition
                    this->inputIsActive = true;
                }
                // now we know the note is not on but we are in active debounce
                // see if debounce interval is satisfied
                if ((millis() - this->activeTStamp) >= this->debounceMsec  ) {
                    this->ctrlIsOn = true;
                    Console_print("note on "); Console_println(noteNum);
                    this->activateControl(noteNum, midiOutChan);
                }
            }
        }
};

