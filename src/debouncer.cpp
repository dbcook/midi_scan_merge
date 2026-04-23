#include <Arduino.h>
#include <MIDI.h>

#include "glob_gen.h"
#include "data.h"
#include "debouncer.h"

void DebouncerMidiNoteSingleContact::activateControl(midi::DataByte noteNum, midi::Channel midiOutChan) {
    gMidiOutputInterface->sendNoteOn(noteNum, midiDefaultVelocity, midiOutChan);
}

void DebouncerMidiNoteSingleContact::deactivateControl(midi::DataByte noteNum, midi::Channel midiOutChan) {
    gMidiOutputInterface->sendNoteOff(noteNum, midiDefaultVelocity, midiOutChan);
}

void DebouncerMidiCCAnalog::setControlValue(midi::DataByte controlNum, midi::Channel midiOutChan) {
    gMidiOutputInterface->sendControlChange(controlNum, scaleInput(filteredCtrlValue), midiOutChan);
}