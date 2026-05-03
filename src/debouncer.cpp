#include <Arduino.h>
#include <MIDI.h>
#include <AppleMIDI.h>

#include "glob_gen.h"
#include "config_features.h"
#include "data.h"
#include "debouncer.h"

// Debouncer action routines
// These use compile time and global transport output settings to generate outbound MIDI messages

void DebouncerMidiNoteSingleContact::activateControl(midi::DataByte noteNum, midi::Channel midiOutChan) {
#if SERIAL_MIDI_OUTPUT
    gMidiSerialOutputInterface->sendNoteOn(noteNum, midiDefaultVelocity, midiOutChan);
#endif
#if ETHERNET_MIDI_OUTPUT
    gMidiEthOutputInterface->sendNoteOn(noteNum, midiDefaultVelocity, midiOutChan);
#endif
}

void DebouncerMidiNoteSingleContact::deactivateControl(midi::DataByte noteNum, midi::Channel midiOutChan) {
#if SERIAL_MIDI_OUTPUT
    gMidiSerialOutputInterface->sendNoteOff(noteNum, midiDefaultVelocity, midiOutChan);
#endif
#if ETHERNET_MIDI_OUTPUT
    gMidiEthOutputInterface->sendNoteOff(noteNum, midiDefaultVelocity, midiOutChan);
#endif
}

void DebouncerMidiCCAnalog::setControlValue(midi::DataByte controlNum, midi::Channel midiOutChan) {
#if SERIAL_MIDI_OUTPUT
    gMidiSerialOutputInterface->sendControlChange(controlNum, scaleInput(filteredCtrlValue), midiOutChan);
#endif
#if ETHERNET_MIDI_OUTPUT
    gMidiEthOutputInterface->sendControlChange(controlNum, scaleInput(filteredCtrlValue), midiOutChan);
#endif
}