#include <Arduino.h>
#include <MIDI.h>

#include "glob_gen.h"
#include "data.h"
#include "debouncer.h"

void DebouncerMidiNoteSingleContact::activateControl() {
    gMidiOutputInterface->sendNoteOn(this->midiNoteNum, midiDefaultVelocity, this->midiOutChan);
}

void DebouncerMidiNoteSingleContact::deactivateControl() {
    gMidiOutputInterface->sendNoteOff(this->midiNoteNum, midiDefaultVelocity, this->midiOutChan);
}

void DebouncerMidiCCAnalog::activateControl() {
    gMidiOutputInterface->sendControlChange(controlNum, scaleInput(filteredCtrlValue), this->midiOutChan);
}