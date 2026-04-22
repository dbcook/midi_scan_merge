#pragma once
#include <Arduino.h>
#include <MIDI.h>
#include "midi_const.h"

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
        DebouncerBase( uint8_t debounceMsec) {
            this->debounceMsec = debounceMsec;
        }

        virtual void activateControl() = 0;
        virtual void deactivateControl() = 0;

        virtual void reset() {
            debounceMsec = 20;
        }

        // Abstraction issues for stateSample - takes different args depending on derived class
        //virtual void stateSample(bool sampleActive) = 0;

};



// Debouncer for analog inputs (pot, rotary encoder, etc.) that generate CC channel messages
class DebouncerMidiCCAnalog : public DebouncerBase {
    protected:
        t_midiInterfaceHWSerialPtr midiInterface;
        midi::DataByte controlNum = 0;      // control number for CC message
        midi::Channel midiOutChan = 0;      // MIDI channel for the emitted CC messages
        int filteredCtrlValue = 0;          // running filtered control value

        // Lowpass filter params and tracking state
        int fuzzTolerance = 0;              // how much sample-to-sample high frequency jitter (in analog input space) is allowed while still considered stable
        int stableTrendThresh = 0;          // how much stable drift is allowed (with successive samples not violating fuzzTolerance) before we send a new CC value
        unsigned long stableTStamp = 0;     // Tms at which the input became stable
        bool inputIsStable = false;         // assume initially unstable so that messages will establish control values at startup

    public:
        DebouncerMidiCCAnalog(uint8_t debounceMsec, t_midiInterfaceHWSerialPtr midiInterface, midi::DataByte controlNum, midi::Channel midiOutChan) : DebouncerBase(debounceMsec) {
            this->midiInterface = midiInterface;
            this->controlNum = controlNum;
            this->midiOutChan = midiOutChan;
        }

        void setFuzzAndTrend(int fuzzTolerance, int stableTrendThresh) {
            this->fuzzTolerance = fuzzTolerance;
            this->stableTrendThresh = stableTrendThresh;
        }

        midi::DataByte scaleInput(int inputVal) {
            return inputVal >> 8;           // *** FIXME ***
        }

        void activateControl() {
           this->midiInterface->sendControlChange(controlNum, scaleInput(filteredCtrlValue), this->midiOutChan);
        }

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

// *** Need to determine RAM consumption of these, want to support up to 256 notes (4 8x8 matrices)
// As they currently exist these debouncers take 20 bytes each (an 8x8 matrix takes up 1280 bytes of RAM)
class DebouncerMidiNoteSingleContact : public DebouncerBase {
    protected:
        uint8_t midiNoteNum = 0;                // *** use callback?  but callback ptr is bigger than a uint8
        t_midiInterfaceHWSerialPtr midiInterface;
        uint8_t midiOutChan = 0;
        const uint8_t midiDefaultVelocity = MIDI_VELOCITY_MAX;
    
        unsigned long activeTStamp = 0;         // time at which the input was first seen in the active state after being inactive
        unsigned long inactiveTStamp = 0;       // time at which the input was first seen in the inactive state after being active
        bool inputIsActive = false;             // true if the last input sample was active
        bool ctrlIsOn = false;                  // true if the debounced control (note, switch, piston, ...) is currently active

    public:
        DebouncerMidiNoteSingleContact(uint8_t debounceMsec, t_midiInterfaceHWSerialPtr midiInterface, uint8_t midiNoteNum, uint8_t midiOutChan) : DebouncerBase(debounceMsec) {
            this->midiNoteNum = midiNoteNum;
            this->midiInterface = midiInterface;
            this->midiOutChan = midiOutChan;
        }

        void activateControl() {
            this->midiInterface->sendNoteOn(this->midiNoteNum, midiDefaultVelocity, this->midiOutChan);
        }

        void deactivateControl() {
            this->midiInterface->sendNoteOff(this->midiNoteNum, midiDefaultVelocity, this->midiOutChan);
        }

        void reset() {
            DebouncerBase::reset();
            activeTStamp = 0;
            inactiveTStamp = 0;
            inputIsActive = false;
            ctrlIsOn = false;
        }

        void stateSample(bool sampleActive) {
            if (sampleActive && this->ctrlIsOn) {
                return;
            }
            if (!sampleActive && !this->ctrlIsOn) {
                return;
            }
            if (sampleActive && !this->inputIsActive) {
                activeTStamp = millis();
                this->inputIsActive = true;
            }
            if (!sampleActive && this->inputIsActive) {
                inactiveTStamp = millis();
                this->inputIsActive = false;
            }
            if (sampleActive) {
                if (this->inputIsActive && !this->ctrlIsOn) {
                    if ((millis() - this->activeTStamp) >= this->debounceMsec  ) {
                        this->ctrlIsOn = true;
                        this->activateControl();
                    }
                }
            }
            else {
                if (!this->inputIsActive && this->ctrlIsOn) {
                    if ((millis() - this->inactiveTStamp) >= this->debounceMsec) {
                        this->ctrlIsOn = false;
                        this->deactivateControl();
                    }
                }
            }
        }

};

