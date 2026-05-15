#include <Arduino.h>
#include <MIDI.h>

#include "glob_gen.h"
#include "data.h"
#include "config_features.h"
#include "debouncer.h"

void DebouncerMidiNoteSingleContact::stateSample(bool sampleActive, midi::DataByte noteNum, midi::Channel midiOutChan) {
    unsigned long curms = millis();
    if (!sampleActive) {
        if (!this->ctrlIsOn) return;    // sample is off, note is off, bail
        // sample is off, but note is on - are we transitioning to release debounce state?
        if (this->inputIsActive) {
            inactiveTStamp = curms;  // record time of transition
            this->inputIsActive = false;
        }
        // now we know note is on but we are in release debounce state
        // see if debounce interval is satisfied
        if ((curms - this->inactiveTStamp) >= this->debounceMsec) {
            this->ctrlIsOn = false;
            Console_print("note off "); Console_println(noteNum);
            this->deactivateControl();
        }
    }
    else {
        if (this->ctrlIsOn) return;     // note is already on, we're done
        // are we transitioning to input-active state (starting debounce)
        if (!this->inputIsActive) {
            activeTStamp = curms;    // record time of transition
            this->inputIsActive = true;
        }
        // now we know the note is not on but we are in active debounce
        // see if debounce interval is satisfied
        if ((curms - this->activeTStamp) >= this->debounceMsec  ) {
            this->ctrlIsOn = true;
            Console_print(F("note on ")); Console_println(noteNum);
            this->activateControl();
        }
    }
}


// Splitting the sample handling to eliminate an arg to the debouncer unexpectedly doubled the scan speed
// based on this I think we need to put the noteNum and outputChannel into the debouncer objects (2 bytes, so costs 0.5KB)
void DebouncerMidiNoteSingleContact::stateSampleInactive() {
    unsigned long curms;
    if (!this->ctrlIsOn) return;    // sample is off, note is off, bail
    // sample is off, but note is on - are we transitioning to release debounce state?
    curms = millis();
    if (this->inputIsActive) {
        inactiveTStamp = curms;  // record time of transition
        this->inputIsActive = false;
    }
    // now we know note is on but we are in release debounce state
    // see if debounce interval is satisfied
    if ((curms - this->inactiveTStamp) >= this->debounceMsec) {
        this->ctrlIsOn = false;
        Console_print("note off "); Console_println(noteNum);
        this->deactivateControl();
    }
}

void DebouncerMidiNoteSingleContact::stateSampleActive() {
    unsigned long curms;
    if (this->ctrlIsOn) return;     // note is already on, we're done
    // are we transitioning to input-active state (starting debounce)
    curms = millis();
    if (!this->inputIsActive) {
        activeTStamp = curms;    // record time of transition
        this->inputIsActive = true;
    }
    // now we know the note is not on but we are in active debounce
    // see if debounce interval is satisfied
    if ((curms - this->activeTStamp) >= this->debounceMsec  ) {
        this->ctrlIsOn = true;
        Console_print(F("note on ")); Console_println(noteNum);
        this->activateControl();
    }
}

// call this instead of the regular stateSample to verify the scanning sequence is correct
// ***better to do this in the scan routine where the scan/read pin numbers are known
void DebouncerMidiNoteSingleContact::stateSampleDummy(bool sampleActive) {
    AM_DBG(F("Ch"), midiOutChan);
    AM_DBG(F("NT"), this->noteNum);
    AM_DBG(F("sample"), sampleActive);
}


// Debouncer action routines
// These use compile time and global transport output settings to generate outbound MIDI messages

void DebouncerMidiNoteSingleContact::activateControl() {
#if SERIAL_MIDI_OUTPUT
    gMidiSerialOutputInterface->sendNoteOn(this->noteNum, midiDefaultVelocity, this->midiOutChan);
#endif
#if ETHERNET_MIDI_OUTPUT
    gMidiEthOutputInterface->sendNoteOn(this->noteNum, midiDefaultVelocity, this->midiOutChan);
#endif
}

void DebouncerMidiNoteSingleContact::deactivateControl() {
#if SERIAL_MIDI_OUTPUT
    gMidiSerialOutputInterface->sendNoteOff(this->noteNum, midiDefaultVelocity, this->midiOutChan);
#endif
#if ETHERNET_MIDI_OUTPUT
    gMidiEthOutputInterface->sendNoteOff(this->noteNum, midiDefaultVelocity, this->midiOutChan);
#endif
}

void DebouncerMidiCCAnalog::setControlValue() {
#if SERIAL_MIDI_OUTPUT
    gMidiSerialOutputInterface->sendControlChange(thiscontrolNum, scaleInput(filteredCtrlValue), this->midiOutChan);
#endif
#if ETHERNET_MIDI_OUTPUT
    gMidiEthOutputInterface->sendControlChange(this->controlNum, scaleInput(filteredCtrlValue), this->midiOutChan);
#endif
}

