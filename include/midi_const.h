#pragma once
#include <Arduino.h>
#include <MIDI.h>
#include <Ethernet3.h>
#include <AppleMIDI.h>
// Various MIDI related constants that don't appear in the library MIDI.h

#define MIDI_VELOCITY_MAX 0x7F
#define MIDI_MAX_NOTES 128              // 0 through 127, defined by MIDI spec

typedef MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>> *  t_midiInterfaceHWSerialPtr;

typedef midi::MidiInterface<appleMidi::AppleMIDISession<EthernetUDP>, appleMidi::AppleMIDISettings> * t_midiInterfaceEthPtr;

typedef appleMidi::AppleMIDISession<EthernetUDP> * t_appleMidiInstancePtr;

// MIDI message types are defined in midi_Defs.h, in enum MidiType.

// MIDI control change (CC) message types are in enum MidiControlChangeNumber
