#pragma once

#include <Arduino.h>
#include <MIDI.h>

#include "glob_gen.h"
#include "config_features.h"

#include "midi_const.h"
#include "pin_list.h"
#include "debouncer.h"

// infinite spin when irrecoverable condition occurs
#define DIE for (;;)

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

// implemented functions are midiMergeThru, midiMerge2, midiMerge3
#if MIDI_MERGE_SERIAL_PORTS == 1
#define MIDI_MERGE_FUNC midiMergeThru
#elif MIDI_MERGE_SERIAL_PORTS == 2
#define MIDI_MERGE_FUNC midiMerge2
#elif MIDI_MERGE_SERIAL_PORTS == 3
#define MIDI_MERGE_FUNC midiMerge3
#else
#endif

#ifdef GEN_GLOBALS
// Set up the MIDI library - compile time macros
// the operation of the macros prevents turning the midi0, midi1, etc. into an indexed array
#if MIDI_MERGE_SERIAL_PORTS > 0
MIDI_CREATE_INSTANCE(HardwareSerial, *serialPorts[0], midi0);
#endif

#if MIDI_MERGE_SERIAL_PORTS > 1
MIDI_CREATE_INSTANCE(HardwareSerial, *serialPorts[1], midi1);
#endif

#if MIDI_MERGE_SERIAL_PORTS > 2
MIDI_CREATE_INSTANCE(HardwareSerial, *serialPorts[2], midi2);
#endif

#endif

#if MIDI_SERIAL_OUTPUT
// ALL MIDI serial output messages go to the first defined serial port
EXTERN t_midiInterfaceHWSerialPtr gMidiSerialOutputInterface
#ifdef GEN_GLOBALS
 = &midi0
#endif
;
#endif

#if ETHERNET_MIDI_CONNECT
// This one has to be initialized at runtime
EXTERN t_midiInterfaceEthPtr gMidiEthOutputInterface
#ifdef GEN_GLOBALS
    = NULL
#endif
;

EXTERN t_appleMidiInstancePtr gAppleMidiInstance
#ifdef GEN_GLOBALS
    = NULL
#endif
;

#endif

// number of active Ethernet AppleMidi connections
EXTERN int gEthConnections
#ifdef GEN_GLOBALS
 = 0
#endif
 ;

// RAM based fixed size array of debouncers - size must be sufficient for all configured diode matrix and parallel pin blocks
// *** oops, can't have array of class objects
EXTERN DebouncerMidiNoteSingleContact gDebouncers[MAX_DEBOUNCERS];

// RAM based global array of debouncer base index for each pin block
EXTERN int gDebouncerBases[nPinBlocks];

