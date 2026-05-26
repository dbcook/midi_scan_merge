#pragma once
#include <Arduino.h>

//-----------------------------------------------------------------------
// Configures the scanning of input pins
//-----------------------------------------------------------------------
// Here is where you specify the input pins layout for the scanner.
// We currently support diode matrix arrays as well as linear (aka parallel) blocks of pins.
//
// Analog input support is planned but not yet implemented.
//
// The pin ranges you specify must be contiguous.  In diode matrix pinBlocks, the scan pin range
// and the read pins range must be individually contiguous, but they don't have to be adjacent.
//
// PINS YOU MUST AVOID
// You CANNOT use any of the following pins in any scan range (subject to some conditions):
//    D0-D1 - hardware serial port reserved for bootloader serial, debugger stub, and console debug messages
//    D4  - Ethernet Shield 2 when SD card is used: chip select for SD card
//    D10 - Ethernet Shield 2 whenever present: chip select for WizNet 5500 chip
//    D13 - LED output on most Arduino implementations
//    D14-D15 - Unavailable if you have a MIDI serial shield or debug monitor connected to SER3
//    D20-21 - Unavailable if the LCD I2C display is in use (these shadow the SDA and SCL lines)
//
// For simplicity, unless you are trying to use all the possible pins I would just avoid using anything
// below D14, D16 or D22 depending on your installed hardware options.
//
// It's possible to increase capacity by having multiple diode matrix blocks share the same
// scan pins or read pins (but not both!) as long as everything has the same polarity.  This enables a theoretical maximum
// of seven 8x8 diode matrix blocks on one Grand Central M4 or Arduino Due.
//
// CAVEAT: there are various logical constraints on the pinBlock definitions that are not
// enforced in the code (yet).  These include:
//      HW configuration based forbidden pins as listed above must not be used in any pinBlock for any purpose
//      The scan pins range and read pins range for a diode matrix must not overlap
//      Analog pins must be in range for the specific processor type
//      Maximum MIDI note number must be observed: (base note number + number of notes - 1) must not exceed 127
//      No pin range can extend beyond pin 69 on a large format Arduino board (Grand Central, Due, etc.)
//      If the note ranges for 2 groups overlap, then each group must be sent to a different channel.
//
// Discontiguous pin ranges for a diode matrix are not yet supported.  However you can map
// arbitrary pins as parallel inputs just by creating multiple parallel blocks that are
// individually contiguous.
//
// For each group, in addition to the Arduino pin ranges you get to specify:
//   The total number of notes to scan (for diode matrix where you likely will use 61 notes out of 64 in an 8x8 matrix)
//   The base MIDI note number for the group
//   The MIDI output channel for the NoteOn and NoteOff messages.  
//
// This file must define a const and fully initialized array of PinBlock_t named "gFlashPinBlocks". 
// To see the PinBlock_t struct declaration, right-click PinBlock_t below and use "go to definition"
//
// When useSelect is false, you should set the select base pin to LED_BUILTIN and the number of
// select pins to 0.  By design the select range will not be written when useSelect == false,
// but even if some bug causes the first select to be written somehow, it will just blink the LED.
//
// (Why am I this paranoid? Because somebody may come along, create a new diode matrix scan routine
// and forget to include the conditional on useSelect.  Then when a parallel block definition is
// hit they will be writing to pins that should not be written to, causing
// seriously unexpected and hard to analyze behavior.  I've seen that sort of thing before...)
//
// Normally activeLow should be true for open-drain contact closures against a pullup resistor.
//
// You can use constants from midi_instruments.h and elsewhere as desired.
//

// Where to get the PinBlock definitions.  Choices are:
//      flash - copy in hardcoded example/test definitions declared here
//      config - read YAML config from SD card

#define PIN_CONFIG_SOURCE flash

// Constants to make the block defs more readable
#define DIODE_MATRIX true
#define PARALLEL false
#define ACTIVE_LOW true
#define SINGLE_CONTACT 1
#define DOUBLE_CONTACT 2
#define TRIPLE_CONTACT 3
#define ATTACK_DEBOUNCE_MSEC 20
#define RELEASE_DEBOUNCE_MSEC 5

// Example flash resident pin configurations - uncomment only one

// A regular 8x8 single contact keyboard with 61 notes
// const PinBlockMulti_t gFlashPinBlocksMulti[] = {
//    {DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 5, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {16, 24}, {0,0}, {0,0} }}     // 8X8 single contact
// };

// MAX stress test - seven 8x8 single contact 61-note keyboards (duplicated so it would send the same note on 7 channels)
const PinBlockMulti_t gFlashPinBlocksMulti[] = {
    {DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 1, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {38, 46}, {0,0}, {0,0} }}     // 8X8 single contact
   ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 2, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {38, 46}, {0,0}, {0,0} }}     // 8X8 single contact
   ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 3, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {38, 46}, {0,0}, {0,0} }}     // 8X8 single contact
   ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 4, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {38, 46}, {0,0}, {0,0} }}     // 8X8 single contact
   ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 5, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {38, 46}, {0,0}, {0,0} }}     // 8X8 single contact
   ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 6, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {38, 46}, {0,0}, {0,0} }}     // 8X8 single contact
   ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 7, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {38, 46}, {0,0}, {0,0} }}     // 8X8 single contact
};

// A double contact keyboard
// const PinBlockMulti_t gFlashPinBlocksMulti[] = {
//    {DIODE_MATRIX, ACTIVE_LOW, DOUBLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 6, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {16, 24}, {32,40}, {0,0} }}    // 8x8 double contact
// };

// A block of 32 parallel inputs for a pedalboard starting at pin 22
// There are not enough physical input pins to support multi-contact of a full keyboard in parallel mode
// You could however have a dual contact 32-note pedalboard
// const PinBlockMulti_t gFlashPinBlocksMulti[] = {
//    {PARALLEL, ACTIVE_LOW, SINGLE_CONTACT, 0, PEDAL32_MAX_NOTES, PEDAL32_MAX_NOTES, PEDAL_LOW_NOTENUM, 5, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {0, 22}, {0,0}, {0,0} }}     // 8X8 single contact
// };

// ------------------------------
// Analog input pin defs
// ------------------------------
// If you need discontiguous CC message numbers you have to define a separate pin block for each one
// Pin numbers:  On Arduino large-format boards, A0 is pin 54.  You can also use the A0, A1, etc. symbols here if you want.
#define ANALOG_FILTER_ALPHA 0.2
#define ANALOG_DEADBAND 0.5
#define ANALOG_LOW_GUARDBAND 1.0
#define ANALOG_HIGH_GUARDBAND 1.0

const PinBlockAnalogRead_t gFlashPinBlocksAnalogRead [] = {
    { A0, 73, 3, ANALOG_DEADBAND, ANALOG_LOW_GUARDBAND, ANALOG_HIGH_GUARDBAND, ANALOG_FILTER_ALPHA }
};
