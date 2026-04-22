#pragma once
#include <Arduino.h>
#include <MIDI.h>
// Various MIDI related constants that don't appear in the library MIDI.h

#define MIDI_VELOCITY_MAX 0x7F
#define MIDI_MAX_NOTES 128              // 0 through 127, defined by MIDI spec

typedef MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>> *  t_midiInterfaceHWSerialPtr;
