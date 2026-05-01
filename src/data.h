#pragma once

#include <Arduino.h>
#include <MIDI.h>

#include "glob_gen.h"

#include "midi_const.h"
#include "pin_list.h"
#include "debouncer.h"


// The use case for remapping is to make sure that messages from all connected MIDI interfaces
// are sent on different channels.
// This comes into play if you have a stack of keyboards, each of which having a scanner
// that outputs MIDI on the same channel. In order to avoid note collisions you have to
// move each keyboard to a unique channel.

// generally should be same as numPorts unless we consume (decode) some port and don't merge it
// Decision: decoder with merge/remap will not be supported
// Decision: If remap is desired, msgs from all MIDI interfaces will be remapped the same

// how many MIDI serial ports are being merged with MIDI channel separation remapping
// allowable range 0-3, 1 means a single incoming channel.
// *** 0 not tested yet
#define MIDI_MERGE_PORTS 1

// Whether to do any channel separation remapping
#define DO_CHANNEL_SEPARATION_REMAP true

// Pointers to pre-existing HardwareSerial objects (board dependent) on which MIDI messages arrive.
EXTERN HardwareSerial *serialPorts[]
#ifdef GEN_GLOBALS
= {
      &Serial3
    // , &Serial1
    // , &Serial2
}
#endif
;

// number of serial MIDI ports that are getting received and processed
EXTERN const uint8_t numPorts
#ifdef GEN_GLOBALS
 = sizeof(serialPorts) / sizeof(serialPorts[0])
 #endif
 ;


#if DO_CHANNEL_SEPARATION_REMAP

// whether the incoming MIDI port should get remapped
EXTERN const uint8_t remapMidiChans[]
#ifdef GEN_GLOBALS
= {true, true, true}
#endif
;

// Channel to which each incoming serial MIDI port is remapped.
// They must be unique, and different from the channel on which this scanner emits.
EXTERN const uint8_t midiRemappedChans[]
#ifdef GEN_GLOBALS
= {1, 2, 3}
#endif
;

#endif

// implemented functions are midiMergeThru, midiMerge2, midiMerge3
#if MIDI_MERGE_PORTS == 1
#define MIDI_MERGE_FUNC midiMergeThru
#elif MIDI_MERGE_PORTS == 2
#define MIDI_MERGE_FUNC midiMerge2
#elif MIDI_MERGE_PORTS == 3
#define MIDI_MERGE_FUNC midiMerge3
#else
#endif

// Midi output channel for scanned notes
// *** TODO: persist in NVRAM and set/query via sysex msg
#define MERGE_OUTPUT_PORT_MAPPED_CHAN 0

#ifdef GEN_GLOBALS
// Set up the MIDI library - compile time macros
// the operation of the macros prevents turning the midi0, midi1, etc. into an indexed array
#if MIDI_MERGE_PORTS > 0
MIDI_CREATE_INSTANCE(HardwareSerial, *serialPorts[0], midi0);
#endif

#if MIDI_MERGE_PORTS > 1
MIDI_CREATE_INSTANCE(HardwareSerial, *serialPorts[1], midi1);
#endif

#if MIDI_MERGE_PORTS > 2
MIDI_CREATE_INSTANCE(HardwareSerial, *serialPorts[2], midi2);
#endif

#endif

// ALL MIDI output messages go to this serial port
// *** TODO fix for USB/Ethernet output
EXTERN t_midiInterfaceHWSerialPtr gMidiOutputInterface
#ifdef GEN_GLOBALS
 = &midi0
#endif
;

#define MAX_DEBOUNCERS 256


// RAM based fixed size array of debouncers - size must be sufficient for all configured diode matrix and parallel pin blocks
// *** oops, can't have array of class objects
EXTERN DebouncerMidiNoteSingleContact gDebouncers[MAX_DEBOUNCERS];

// RAM based global array of debouncer base index for each pin block
EXTERN int gDebouncerBases[nPinBlocks];

