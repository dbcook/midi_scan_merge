#pragma once

#include <Arduino.h>
#include <MIDI.h>
#include <AppleMIDI.h>
#include <LiquidCrystal_I2C.h>

#include "glob_gen.h"
#include "debug.h"
#include "deque.h"
#include "lcd_display.h"

// USE_EXT_CALLBACKS is necessary to catch any MIDI events
// Since it only takes about 150 bytes of flash we turn it on by default.
#define USE_EXT_CALLBACKS


#include "config_features.h"

#include "midi_const.h"
#include "pin_list.h"
#include "debouncer.h"

EXTERN const char * gProdName
#ifdef GEN_GLOBALS
= "MIDI Processor"
#endif
;

EXTERN const char * gProdVersion
#ifdef GEN_GLOBALS
= "0.9.1.2"
#endif
;

EXTERN const char * gProdCopyright
#ifdef GEN_GLOBALS
= "Copyright 2026 by David B. Cook"
#endif
;

EXTERN const char * gProdLicense
#ifdef GEN_GLOBALS
= "Apache 2.0 license.  See LICENSE for details."
#endif
;

// The use case for MIDI channel remapping is to make sure that messages from all connected MIDI interfaces
// are sent on different channels.
// This comes into play if you have a stack of keyboards, each of which having a scanner
// that outputs MIDI on the same channel. In order to avoid note collisions you have to
// move each keyboard to a unique channel.

// generally should be same as numPorts unless we consume (decode) some port and don't merge it
// Decision: serial input decoder port with other ports doing merge/remap will not be supported
// Decision: If remap is desired, msgs from all MIDI interfaces will be remapped the same

// how many MIDI serial ports are being merged with MIDI channel separation remapping
// allowable range 0-3, 1 means a single incoming channel.

// implemented functions are midiMergeThru, midiMerge2, midiMerge3
// ***TODO we need a function pointer type and a configured pointer var
#if MIDI_MERGE_SERIAL_PORTS == 1
#define MIDI_MERGE_FUNC midiMergeThru
#elif MIDI_MERGE_SERIAL_PORTS == 2
#define MIDI_MERGE_FUNC midiMerge2
#elif MIDI_MERGE_SERIAL_PORTS == 3
#define MIDI_MERGE_FUNC midiMerge3
#else
#endif


#if MIDI_SERIAL_OUTPUT
// ALL MIDI serial output messages go to the first defined serial port
EXTERN t_midiInterfaceHWSerialPtr gMidiSerialOutputInterface
#ifdef GEN_GLOBALS
 = &midi0
#endif
;
#endif

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

// number of active Ethernet AppleMidi connections
EXTERN int gEthConnections
#ifdef GEN_GLOBALS
 = 0
#endif
 ;

// RAM based fixed size array of debouncers - size must be sufficient for all configured diode matrix and parallel pin blocks
EXTERN DebouncerMidiNoteSingleContact gDebouncers[MAX_DEBOUNCERS];

// RAM based global array of debouncer base index for each pin block - it will rarely exceed 10-15
EXTERN Deque<int> gDebouncerBases
#ifdef GEN_GLOBALS
(15)
#endif
;


// ***TODO should become part of the global config object

EXTERN bool gUseSerialMidi
#ifdef GEN_GLOBALS
    = SERIAL_MIDI_INPUT || SERIAL_MIDI_OUTPUT
#endif
;
