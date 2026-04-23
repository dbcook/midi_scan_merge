#pragma once
#include <Arduino.h>
#include <MIDI.h>
// Various MIDI related constants that don't appear in the library MIDI.h

#define MIDI_VELOCITY_MAX 0x7F
#define MIDI_MAX_NOTES 128              // 0 through 127, defined by MIDI spec

typedef MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>> *  t_midiInterfaceHWSerialPtr;

// MIDI message types and masks not used in this code but helpful for reference
//  *** These are likely defined in the 47Effects MIDI library, remove when found ***

// Status byte values.  Except as indicated, all are followed by 2 data bytes.
#define MIDI_STATUS_MASK 0x80
#define MIDI_CHANNEL_MASK 0x0F

// All system messages have no channel number; instead having extended opcodes
// system common messages
#define MIDI_SYSTEM 0xF0                // has subclasses based on low nibble
#define MIDI_SYSEX 0xF0                 // variable length, terminated by MIDI_EOX, 7 bit encoded
#define MIDI_EOX 0xF7                   // end of SYSEX marker
#define MIDI_SYS_MTC 0xF1               // MIDI Time Code, 1 byte
#define MIDI_SYS_SONGPOS 0xF2           // Song Position Pointer, 2 bytes
#define MIDI_SYS_SONGSEL 0xF3           // Song select, 1 byte
#define MIDI_SYS_CABLESEL 0xF5          // Cable select, 1 byte
#define MIDI_SYS_TUNEREQ 0xF6           // Tune request, 0 bytes

// system realtime, all with 0 data bytes
#define MIDI_SYS_TIMING 0xF8            // Timing clock, sent at 24ppqn when clock is running
#define MIDI_SYS_START 0xFA             // Start - clock starts
#define MIDI_SYS_CONTINUE 0xFB          // Continue - clock continues after stop
#define MIDI_SYS_STOP 0xFC              // Stop - when clock stops
#define MIDI_SYS_ACTIVESENSING 0xFE     // Active sensing, sent by some devices when idle to indicate connection is alive
#define MIDI_SYS_RESET 0xFF             // System reset - resets all devices to power-up state = panic button


#define MIDI_NOTE_ON 0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_POLY_PRESSURE 0xA0
#define MIDI_CONTROL_CHANGE 0xB0
#define MIDI_PROGRAM_CHANGE 0xC0        // 1 data byte
#define MIDI_CHANNEL_PRESSURE 0xD0      // 1 data byte
#define MIDI_PITCH_BEND 0xE0

