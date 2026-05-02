#include <Arduino.h>
#include <MIDI.h>

#include "glob_gen.h"
#include "data.h"
#include "debouncer.h"

void DebouncerMidiNoteSingleContact::activateControl(midi::DataByte noteNum, midi::Channel midiOutChan) {
    gMidiSerialOutputInterface->sendNoteOn(noteNum, midiDefaultVelocity, midiOutChan);
}

void DebouncerMidiNoteSingleContact::deactivateControl(midi::DataByte noteNum, midi::Channel midiOutChan) {
    gMidiSerialOutputInterface->sendNoteOff(noteNum, midiDefaultVelocity, midiOutChan);
}

void DebouncerMidiCCAnalog::setControlValue(midi::DataByte controlNum, midi::Channel midiOutChan) {
    gMidiSerialOutputInterface->sendControlChange(controlNum, scaleInput(filteredCtrlValue), midiOutChan);
}