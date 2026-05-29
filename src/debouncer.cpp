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
        if ((curms - this->inactiveTStamp) >= this->releaseDebounceMsec) {
            this->ctrlIsOn = false;
            AM_DBG(F("note off"), noteNum);
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
        if ((curms - this->activeTStamp) >= this->attackDebounceMsec  ) {
            this->ctrlIsOn = true;
            AM_DBG(F("note on"), noteNum);
            this->activateControl();
        }
    }
}


// Splitting the sample handling to eliminate an arg to the debouncer unexpectedly doubled the scan speed
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
    // see if release debounce interval is satisfied
    if ((curms - this->inactiveTStamp) >= this->releaseDebounceMsec) {
        this->ctrlIsOn = false;
        AM_DBG(F("note off"), noteNum);
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
    if ((curms - this->activeTStamp) >= this->attackDebounceMsec  ) {
        this->ctrlIsOn = true;
        AM_DBG(F("note on"), noteNum);
        this->activateControl();
    }
}


// Debouncer action routines
// These use global transport output settings to generate outbound MIDI messages

void DebouncerMidiNoteSingleContact::activateControl() {
#if SERIAL_MIDI_OUTPUT
    // *** still compile time protected b/c output gMidiSerialOutputIinterface isn't always defined
    if (gConfig.useSerialScanOutput) {
        gMidiSerialOutputInterface->sendNoteOn(this->noteNum, midiDefaultVelocity, this->midiOutChan);
    }
#endif
    if (gConfig.useEthernetOutput) {
        gMidiEthOutputInterface->sendNoteOn(this->noteNum, midiDefaultVelocity, this->midiOutChan);
    }
}

void DebouncerMidiNoteSingleContact::deactivateControl() {
#if SERIAL_MIDI_OUTPUT
    if (gConfig.useSerialScanOutput) {
        gMidiSerialOutputInterface->sendNoteOff(this->noteNum, midiDefaultVelocity, this->midiOutChan);
    }
#endif
    if (gConfig.useEthernetOutput) {
        gMidiEthOutputInterface->sendNoteOff(this->noteNum, midiDefaultVelocity, this->midiOutChan);
    }
}

void DebouncerMidiCCAnalog::setControlValue() {
#if SERIAL_MIDI_OUTPUT
    if (gConfig.useSerialScanOutput) {
        gMidiSerialOutputInterface->sendControlChange(thiscontrolNum, scaleInput(filteredCtrlValue), this->midiOutChan);
    }
#endif
    if (gConfig.useEthernetOutput) {
        gMidiEthOutputInterface->sendControlChange(this->controlNum, scaleInput(filteredCtrlValue), this->midiOutChan);
    }
}

