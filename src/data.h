#pragma once

#include <Arduino.h>
#include <MIDI.h>

#include "glob_gen.h"

#include "midi_const.h"
#include "pin_list.h"
#include "debouncer.h"


// how many MIDI serial ports will be merged with MIDI channel remapping
// allowable range 1-3, 1 means a single channel which implies no merging, does a channel-remapped thru only
// generally should be same as numPorts unless we consume some channel and don't merge it
#define MIDI_MERGE_CHANS 1

// flexible remapping is not implemented yet; this is simplistic
EXTERN const uint8_t remapMidiChans[]
#ifdef GEN_GLOBALS
= {true, true, true}
#endif
;

EXTERN const uint8_t midiRemappedChans[]
#ifdef GEN_GLOBALS
= {1, 2, 3}
#endif
;

// Pointers to pre-existing HardwareSerial objects (board dependent).  Set these to be the desired set of ports to be serviced by the MIDI RX/merge engine.
EXTERN HardwareSerial *serialPorts[]
#ifdef GEN_GLOBALS
= {
      &Serial3
    // , &Serial1
    // , &Serial2
}
#endif
;
EXTERN const uint8_t numPorts
#ifdef GEN_GLOBALS
 = sizeof(serialPorts) / sizeof(serialPorts[0])
 #endif
 ;

// implemented functions are midiMergeThru, midiMerge2, midiMerge3
#if MIDI_MERGE_CHANS == 1
#define MIDI_MERGE_FUNC midiMergeThru
#elif MIDI_MERGE_CHANS == 2
#define MIDI_MERGE_FUNC midiMerge2
#elif MIDI_MERGE_CHANS == 3
#define MIDI_MERGE_FUNC midiMerge3
#else
#endif

// Midi output channel for scanned notes
// *** TODO: persist in NVRAM and set/query via sysex msg
#define MERGE_OUTPUT_PORT_MAPPED_CHAN 0

#ifdef GEN_GLOBALS
// Set up the MIDI library - compile time macros
// the operation of the macros prevents turning the midi0, midi1, etc. into an indexed array
MIDI_CREATE_INSTANCE(HardwareSerial, *serialPorts[0], midi0);

#if MIDI_MERGE_CHANS > 1
MIDI_CREATE_INSTANCE(HardwareSerial, *serialPorts[1], midi1);
#endif

#if MIDI_MERGE_CHANS > 2
MIDI_CREATE_INSTANCE(HardwareSerial, *serialPorts[2], midi2);
#endif

#endif

// ALL MIDI output messages go to this port
EXTERN t_midiInterfaceHWSerialPtr gMidiOutputInterface
#ifdef GEN_GLOBALS
 = &midi0
#endif
;

#define MAX_DEBOUNCERS 256


// RAM based fixed size array of debouncers - size must be sufficient for all configured diode matrix and parallel pin blocks
EXTERN DebouncerMidiNoteSingleContact gDebouncers[MAX_DEBOUNCERS];

// RAM based global array of debouncer base index for each pin block
EXTERN int gDebouncerBases[nPinBlocks];

