#pragma once
#include <Arduino.h>
#include "glob_gen.h"

/*
    Configure global features for the MIDI scanner / merger / decoder
*/

// Major features

// What channel scanned note and CC messages are sent on (independent of transport)
// *** TODO: persist in NVRAM and set/query via sysex msg
#define MIDI_SCAN_OUTPUT_CHAN 0

// Contact debouncer allocation
// The largest this could ever be would be 8*64 = 512, which would support 7 8x8 diode matrix
// keyboards / pedalboards / piston arrays, with the 8 select lines shared across all matrices.
// On Arduino Due and Teensy 4.1 this will be no problem.
// On a mega you can't go much past 256 and maybe not even that far due to both memory and CPU limitations.

#if defined(ARDUINO_AVR_DUEMILANOVE)
#define MAX_DEBOUNCERS 512
#elif defined(ARDUINO_AVR_MEGA2560)
#define MAX_DEBOUNCERS 256
#else
#error "Unsupported processor type!"
#endif



//---------------------------------------
// Serial MIDI Interfaces
//---------------------------------------


#define MERGE_SERIAL_INPUTS true

#if MERGE_SERIAL_INPUTS

// Number of serial MIDI ports to read and merge
#define MIDI_MERGE_SERIAL_PORTS 1

// Whether to do any channel separation remapping when merging serial MIDI input streams
#define DO_CHANNEL_SEPARATION_REMAP true

// MIDI output channel for merge/thru channel msgs
// Should be same as MIDI_SCAN_OUTPUT_CHAN except in rare cases
#define MERGE_OUTPUT_PORT_MAPPED_CHAN MIDI_SCAN_OUTPUT_CHAN

// Pointers to system defined HardwareSerial objects (number of them is target dependent)
// on which MIDI messages arrive.
// Only define ports that will be used; no duplicates allowed.
// MIDI interfaces midi0, midi1, midi2 will be assigned to these ports in order
// Output MIDI serial messages will all go to the FIRST defined port (which becomes midi0)
// The order of Serial3, Serial2, Serial1 maximimizes contiguous IO pins on Arduino Due and Mega.
EXTERN HardwareSerial *serialPorts[]
#ifdef GEN_GLOBALS
= {
#if MIDI_MERGE_SERIAL_PORTS == 1
      &Serial3
    // , &Serial2
    // , &Serial1
#elif MIDI_MERGE_SERIAL_PORTS == 2
      &Serial3
    , &Serial2
    // , &Serial1
#elif MIDI_MERGE_SERIAL_PORTS == 3
      &Serial3
    , &Serial2
    , &Serial1
#else
#error MIDI_MERGE_SERIAL_PORTS has illegal value!
#endif
}
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

#endif // DO_CHANNEL_SEPARATION_REMAP

// Number of serial MIDI RX ports that are getting processed
EXTERN const uint8_t numPorts
#ifdef GEN_GLOBALS
 = sizeof(serialPorts) / sizeof(serialPorts[0])
 #endif
 ;

#endif // MERGE_SERIAL_INPUTS

//---------------------------------------
// Ethernet MIDI Interfaces
//---------------------------------------

#define ETHERNET_MIDI_CONNECT true
#define ETHERNET_MIDI_OUTPUT true

#if ETHERNET_MIDI_CONNECT

#define ETH_HOSTNAME_PREFIX "dbc_midiproc"

// This transport instance name has to be kept unique vs the serial and USB transport names
#define ETH_MIDI_NAME ETHMIDI

// MIDI channel the Ethernet port listens on for decoding and commands
// Set to all chans since we assume that everything sent to our session is for us
#define ETH_MIDI_LISTEN_CHAN MIDI_CHANNEL_OMNI

// Newer Ethernet shields have a MAC address printed on a sticker on the shield
// *** Teensy 4.1 Ethernet MAC determination TBD
// minor bug: lib declaration of Ethernet.begin won't allow const uint8_t * so can't put this into flash
EXTERN uint8_t gEthernetMac[]
#ifdef GEN_GLOBALS
 = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xF0, 0x0D
}
#endif
;

#endif // ETHERNET_MIDI_CONNECT