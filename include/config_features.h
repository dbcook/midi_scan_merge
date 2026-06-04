#pragma once
#include <Arduino.h>
#include "glob_gen.h"
#include <variant.h>
#include <MIDI.h>
#include "ArrayList.h"

#include "serial_gcm4.h"

/*
    Configure global features for the MIDI scanner / merger / decoder
*/

// Major features

// Contact debouncer allocation
// The largest this could ever be without additional IO hardware would be 7*64 = 448, which would support 7 8x8 diode matrix
// keyboards / pedalboards / piston arrays, with the 8 select lines shared across all matrices.
// On Grand Central, Arduino Due and Teensy 4.1 this amount of memory will be no problem.

#define MAX_DEBOUNCERS 448

//---------------------------------------
// Configuration struct
//---------------------------------------

typedef struct runtimeConfig {
    bool isProductionBuild;           // can be used to generally disable non-production features
    bool useLcd;                      // whether to write to LCD display.  OK even if LCD is not attached but with ~1% performance loss.
    bool logScanSequence;             // true disables regular scanning and logs the whole sequence to console every 10 sec
    bool useEthernet;                 // true if we run Ethernet AppleMIDI session connect listener
    bool useEthernetOutput;           // true if MIDI is to be sent outbound on Ethernet
    bool useUSB;                      // true if USB-MIDI transport is enabled
    bool useSerialScanOutput;         // true for serial MIDI output from scan
    bool showFreeMem;                 // true to display free memory on console and LCD lower left corner.  Slight cost in scan rate.
    char ethernetMac[6];              // Ethernet Mac Address
    uint8_t numSerialMergeChans;      // Number of serial MIDI merge channels, 0 to disable
    int matrixStabilizationUsec;      // stabilization delay in microseconds following select line writes.  Must be at least 7-10 usec.

} runtimeConfig_t;


// Default runtime configuration.  These parameters can be overridden by the config file loaded from SD card or QSPI drive.
EXTERN runtimeConfig_t gConfig
#ifdef GEN_GLOBALS
PROGMEM = {
    true,                                     // Production build
    true,                                     // use LCD
    false,                                    // scan sequence logging (DISABLES REGULAR SCAN)
    true,                                     // run Ethernet connections
    true,                                     // send MIDI on Ethernet transport.  Requires useEthernet.
    false,                                    // run USB transport
    false,                                    // run serial transport output from hw input scanning
    false,                                    // show free memory on console and LCD
    { 0xDE, 0xAD, 0xBE, 0xEF, 0xF0, 0x0D },   // Ethernet MAC address
    0,                                        // Number of serial MIDI merge input channels
    10                                        // Stabilization delay, microseconds
  }
#endif
;


//---------------------------------------
// Serial MIDI Interfaces
//---------------------------------------

// *** working on allowing serial output without reading any input ports
// all combinations of SERIAL_MIDI_INPUT and SERIAL_MIDI_OUTPUT are valid
//

#define SERIAL_MIDI_INPUT false
#define SERIAL_MIDI_OUTPUT false

// Create the serial MIDI output channel and take its address for later use


// Configure serial input port count and merging
// if we have active MIDI serial input ports, assume for now that we are merging them all
// ***FEATURE (low priority) add support for having ports+channels that we consume and other that are forwarded
#if SERIAL_MIDI_INPUT
#define MIDI_SERIAL_INPUT_PORTS 1
#define MERGE_SERIAL_INPUTS true
#endif // SERIAL_MIDI_INPUT

// Configure number of serial input ports to be merged onto the output transport(s)
#if MERGE_SERIAL_INPUTS
#define MIDI_MERGE_SERIAL_PORTS 1
#if MIDI_MERGE_SERIAL_PORTS > MIDI_SERIAL_INPUT_PORTS
#error MIDI_MERGE_SERIAL_PORTS must be <= MIDI_SERIAL_INPUT_PORTS
#endif
#endif

// Configure serial output port count
#if SERIAL_MIDI_OUTPUT
#define MIDI_SERIAL_OUTPUT_PORTS 1
#if MIDI_SERIAL_OUTPUT_PORTS > 1
#error MIDI_SERIAL_OUTPUT_PORTS > 1 unsupported!
#endif
#endif // SERIAL_MIDI_OUTPUT

// Configure array of port addresses needed
#if SERIAL_MIDI_INPUT || SERIAL_MIDI_OUTPUT

// compute total number of serial ports = max(input, output)
#if MIDI_SERIAL_INPUT_PORTS > MIDI_SERIAL_OUTPUT_PORTS
#define NUM_SERIAL_PORTS MIDI_SERIAL_INPUT_PORTS
#else
#define NUM_SERIAL_PORTS MIDI_SERIAL_OUTPUT_PORTS
#endif


#endif // SERIAL_MIDI_INPUT || SERIAL_MIDI_OUTPUT

#if MERGE_SERIAL_INPUTS

// Whether to do any channel separation remapping when merging serial MIDI input streams.
// Channel separation is required if there is any possible collision
// where a [channel, notenum] tuple is duplicated between two sources.
//
// Generally when merging daisy chained inputs in the organ world we will need to do
// per-source MIDI channel separation since there are a lot of bespoke keyboard scanners 
// out there on which you cannot conveniently set the output channel.

#define DO_CHANNEL_SEPARATION_REMAP true

// Pointers to system defined HardwareSerial objects (number of them is target dependent)
// on which MIDI messages arrive.
// Only define ports that will be used; no duplicates allowed.
// MIDI interfaces midi0, midi1, midi2 will be assigned to these ports in order
//
// Output MIDI serial messages will all go to the FIRST defined port (which becomes midi0)
// The order of Serial3, Serial2, Serial1 maximimizes contiguous IO pins on Arduino large-format boards.

#if DO_CHANNEL_SEPARATION_REMAP

// whether the incoming MIDI port should get remapped
EXTERN const uint8_t remapMidiChans[NUM_SERIAL_PORTS]
#ifdef GEN_GLOBALS
= {true}
#endif
;

// Channel to which each incoming serial MIDI port is remapped.
// They must be unique, and different from the channel on which this scanner emits.
EXTERN const uint8_t midiRemappedChans[NUM_SERIAL_PORTS]
#ifdef GEN_GLOBALS
= {1}
#endif
;

#endif // DO_CHANNEL_SEPARATION_REMAP

#endif // MERGE_SERIAL_INPUTS

//---------------------------------------
// Ethernet MIDI Interfaces
//---------------------------------------

#define ETHERNET_MIDI_CONNECT true
#define ETHERNET_MIDI_OUTPUT true
#define ETHERNET_MIDI_DECODE_INPUT true

#if ETHERNET_MIDI_CONNECT

#define ETH_HOSTNAME_PREFIX "dbc_midiproc"

// This transport instance names generated from this have to be kept unique vs the serial and USB transport names
// It MUST resolve to a compile-time literal, not a string constant
#define ETH_MIDI_BASENAME MIDI
#define ETH_APPLE_BASENAME AppleETHMIDI

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

//---------------------------------------
// USB MIDI Interfaces
//---------------------------------------

#define USB_MIDI_CONNECT false

#if USB_MIDI_CONNECT

#endif

