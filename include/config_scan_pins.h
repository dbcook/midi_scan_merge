#pragma once
#include <Arduino.h>

//-----------------------------------------------------------------------
// Scanning configuration for digital and analog input pins
//-----------------------------------------------------------------------
// Here is where you specify the input pins layout for the scanner.
// We currently support diode matrix arrays as well as linear (aka parallel) blocks of digital pins.
//
// Analog input support is planned but not yet fully implemented.
//
// The read pin ranges you specify must be contiguous, whether for parallel or diode matrix inputs.
// Discontiguous pin ranges for the select pins in a diode matrix can be supported by defining a separate
// pin block for each clump of adjacent select pins.  The range of read pins must always be contiguous
// since that is crucial to scanning efficiency.
//
// PINS YOU MUST AVOID
// You CANNOT use any of the following pins in any scan range (subject to some conditions):
//    D0-D1 - hardware serial port reserved for bootloader serial, debugger stub, and console debug messages
//    D4  - Ethernet Shield 2 when SD card is used: chip select for SD card
//    D10 - Ethernet Shield 2 whenever present: chip select for WizNet 5500 chip
//    D13 - LED output on most Arduino implementations
//    D14-D15 - Unavailable if you have a MIDI serial shield or debug monitor connected to SER3
//    D20-21 - Unavailable if the LCD I2C display is in use (these shadow the SDA and SCL lines)
//    D50-52 - Unavailable if the Ethernet shield is in use (MISO MOSI SCK for the primary SPI bus)
//    D62-66 - These don't exist on the Grand Central and trying to reference them crashes
//
// For simplicity, unless you are trying to use all the possible pins I would just avoid using anything
// below D14, D16 or D22 depending on your installed hardware options.
//
// CAUTION AOBUT DISJOINT HIGH PIN RANGES ON THE GRAND CENTRAL
// On Arduino Due, A0 is pin 54 and they go up from there to A11 = pin 65.  This is good.
// You can always safely use the A0, A1, etc. symbols for Due as long as you don't exceed A11 == D65.
//
// However...
//
// On Grand Central, the analog pins are in disjoint blocks and there is a gap in the high digital pins!
// A0-A7 are digital 67-74.  A8-A15 are digital 54-61.  Trying to reference digital 62-66 causes a hard crash.
// Be careful when using the Axx symbols that you do not make a block that references digital pins 62-66 in any way.
// Be especially vigilant about not making any pin range that crosses the A7-A8 boundary since this will violate the gap.
//
// It's possible to increase capacity by having multiple diode matrix blocks share the same
// scan pins or read pins (but not both!) as long as everything has the same polarity.  This enables a theoretical maximum
// of seven 8x8 diode matrix blocks on one Grand Central M4 or Arduino Due.
//
// Diode matrix arrays structured as 8x8 (select lines x read lines) will work fine, but performance could
// improve substantially it were possible to use more read lines per select line, due to the stabilization
// delay needed of ~10 microseconds following each select line write.  It's not yet supported but we will
// eventually be able to set up a double-contact keyboard as an 8x16 matrix.
//
// To create a general flash configuration that serve a variety of configurations, keep in mind that
// there is no harm in scanning inputs that aren't connected.  On a Grand Central the processing is so
// fast that the extra work doesn't matter.  You could set up a configuration with 4 8x8 matrix keyboards
// using shared scan lines (40 inputs) plus a 4x8 pedalboard (12 inputs) and a few analog inputs for
// swell pedals, and use that for 1-4 keyboards, any 27-32 note pedalboard (or none), and 0 to half a
// dozen expression pedals.
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
// For each group, in addition to the Arduino pin ranges you get to specify:
//   The total number of notes to scan (for diode matrix where you likely will use 61 notes out of 64 in an 8x8 matrix)
//   The base MIDI note number for the group
//   The MIDI output channel for the NoteOn and NoteOff messages.  
//
// This file must define a const and fully initialized array of PinBlockMulti_t named "gFlashPinBlocksMulti". 
// To see the PinBlockMulti_t struct declaration, right-click PinBlockMulti_t below and use "go to definition"
//
// When useSelect is false (parallel), you should set the select base pin to LED_BUILTIN and the number of
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

const uint8_t MidiChan_0 = 0;
const uint8_t MidiChan_1 = 1;
const uint8_t MidiChan_2 = 2;
const uint8_t MidiChan_3 = 3;
const uint8_t MidiChan_4 = 4;
const uint8_t MidiChan_5 = 5;
const uint8_t MidiChan_6 = 6;
const uint8_t MidiChan_7 = 7;
const uint8_t MidiChan_8 = 8;
const uint8_t MidiChan_9 = 9;
const uint8_t MidiChan_10 = 10;
const uint8_t MidiChan_11 = 11;
const uint8_t MidiChan_12 = 12;
const uint8_t MidiChan_13 = 13;
const uint8_t MidiChan_14 = 14;
const uint8_t MidiChan_15 = 15;

// Example flash resident pin configurations - uncomment only one

// A regular 8x8 single contact keyboard with 61 notes
// const PinBlockMulti_t gFlashPinBlocksMulti[] = {
//    {DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, MidiChan_5, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {22, 30}, {0,0}, {0,0} }}     // 8X8 single contact
// };

// Max stress test - seven 8x8 single contact 61-note keyboards (duplicated so it would send the same note on 7 channels)
// Scan rate is 1.0 KHz on Grand Central with 10 usec stabilization time
// FOR PERFORMANCE TESTING ONLY
// This many inputs is physically achievable with 64 free pins though you would have to break up the block for the low pins and separate the pin ranges
// const PinBlockMulti_t gFlashPinBlocksMulti[] = {
//     {DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, MidiChan_1, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {30, 38}, {0,0}, {0,0} }}
//    ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, MidiChan_2, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {30, 38}, {0,0}, {0,0} }}
//    ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, MidiChan_3, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {30, 38}, {0,0}, {0,0} }}
//    ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, MidiChan_4, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {30, 38}, {0,0}, {0,0} }}
//    ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, MidiChan_5, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {30, 38}, {0,0}, {0,0} }}
//    ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, MidiChan_6, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {30, 38}, {0,0}, {0,0} }}
//    ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, MidiChan_7, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {30, 38}, {0,0}, {0,0} }}
// };

// full organ rate test: 4 keyboard 8x8, 1 piston 8x12, 1 pedalboard 4x8
const PinBlockMulti_t gFlashPinBlocksMulti[] = {
    {DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, MidiChan_6, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {22, 40}, {0,0}, {0,0} }}
    ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, MidiChan_7, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {22, 40}, {0,0}, {0,0} }}
    ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, MidiChan_8, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {22, 40}, {0,0}, {0,0} }}
    ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, MidiChan_9, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {22, 40}, {0,0}, {0,0} }}
    ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 12, 96, PISTONS_LOW_CTRLNUM, MidiChan_10, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {22, 30}, {0,0}, {0,0} }}
    ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 4, 8, PEDAL32_MAX_NOTES, PEDAL_LOW_NOTENUM, MidiChan_11, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {22, 40}, {0,0}, {0,0} }}
//  ,{DIODE_MATRIX, ACTIVE_LOW, SINGLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, 68, MidiChan_6, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {22, 40}, {0,0}, {0,0} }}     // fail test - notenum hits 128
};

// A double contact keyboard
// const PinBlockMulti_t gFlashPinBlocksMulti[] = {
//    {DIODE_MATRIX, ACTIVE_LOW, DOUBLE_CONTACT, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, MidiChan_5, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {22,30}, {38,53}, {0,0} }} // dodge forbidden pins 50-52 (eth SPI)
// };

// A block of 24 parallel inputs for a pedalboard starting at pin 22
// There are not enough physical input pins to support multi-contact of a full keyboard in parallel mode
// You could however have a dual contact 32-note pedalboard if not using Ethernet, thus making pins 50-52 available
// const PinBlockMulti_t gFlashPinBlocksMulti[] = {
//    {PARALLEL, ACTIVE_LOW, SINGLE_CONTACT, 0, 24, 24, PEDAL_LOW_NOTENUM, MidiChan_5, ATTACK_DEBOUNCE_MSEC, RELEASE_DEBOUNCE_MSEC, { {0, 22}, {0,0}, {0,0} }}     // 8X8 single contact
// };

// ------------------------------
// Analog input pin defs
// ------------------------------
// If you need discontiguous CC message numbers you have to define a separate pin block for each one
// See caution at the top of the file about pin blocks NOT ever using pins 62-66 on the Grand Central.
#define ANALOG_FILTER_ALPHA 0.2
#define ANALOG_DEADBAND 0.5
#define ANALOG_LOW_GUARDBAND 1.0
#define ANALOG_HIGH_GUARDBAND 1.0

const PinBlockAnalogRead_t gFlashPinBlocksAnalogRead [] = {
    { 54, 4, MidiChan_5, 73, ANALOG_DEADBAND, ANALOG_LOW_GUARDBAND, ANALOG_HIGH_GUARDBAND, ANALOG_FILTER_ALPHA }
//  { 60, 3, MidiChan_5, 73, ANALOG_DEADBAND, ANALOG_LOW_GUARDBAND, ANALOG_HIGH_GUARDBAND, ANALOG_FILTER_ALPHA }    // fail test - pin 62 illegal on Grand Central
};
