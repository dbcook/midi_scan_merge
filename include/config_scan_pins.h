#pragma once

//-----------------------------------------------------------------------
// Configures the scanning of input pins
//-----------------------------------------------------------------------
// Here is where you specify the diode matrix and parallel input pins layout for the scanner.
//
// For each group, in addition to the Arduino input pin ranges you get to specify:
//   The total number of notes to scan (for diode matrix where you likely will use 61 notes out of 64 in an 8x8 matrix)
//   The base MIDI note number for the group
//   The MIDI output channel for the NoteOn and NoteOff messages.  
//      If the note ranges for 2 groups overlap, then each group must be sent to a different channel.
//
// This file must define a const and fully initialized array of PinBlock_t named "gPinBlocks". 
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
// Normally activeLow should be true for contact closures against a pullup resistor.
//
// You can use constants from midi_instruments.h and elsewhere as desired.
//
// You CANNOT use any of the following pins in any scan range:
//    D0-D1 - hardware serial port reserved for bootloader serial and console debug messages
//    D4  - Ethernet Shield 2: chip select for SD card
//    D10 - Ethernet Shield 2: chip select for WizNet 5500 chip
//    D13 - LED output on most Arduino implementations
//    D14-D15 - MIDI serial shield connected to SER3 
//
// If you don't have a MIDI serial shield you can freely use all pins from D14 to D53 as well as all 16 analog pins,
// giving 56 readily available inputs, plus 3 short sequences (D2-3, D5-9, D11-12) that could be defined as
// parallel blocks.  That will allow three 8x8 diode matrix keyboards plus 9 discretes (pistons etc.)
//
// Discontiguous pin ranges for a diode matrix are not yet supported.  To avoid consuming too much memory with a general
// in-memory table allowing general discontiguous pins, it will need to be done in code.  A likely
// way to do this is by recognizing a special base pin (pin 2 probably) that will programmatically
// scan a hardcoded 8x8 (keyboard) or 8x4 (pedal) matrix sequence that dodges the pins that must be avoided.
// This will not be necessary for parallel inputs as they can just be defined in short ranges to avoid the unusable pins.

// A few constants to make the block defs more readable
#define DIODE_MATRIX true
#define PARALLEL false
#define ACTIVE_LOW true

// A single 8x8 keyboard matrix, 61 notes, starting on pin 16, output to channel 8
// const PinBlock_t gPinBlocks[]  = {
//     {DIODE_MATRIX,  ACTIVE_LOW, 16, 8, 24, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 8}
// };

// Two 8x8 matrix keyboards, 61 actual notes, starting on pins 16 and 32, output to channels 5 and 6
const PinBlock_t gPinBlocks[]  = {
    {DIODE_MATRIX,  ACTIVE_LOW, 16, 8, 24, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 5}
   ,{DIODE_MATRIX,  ACTIVE_LOW, 32, 8, 40, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 6}
};

// 4x8 pedalboard, 32 notes, starting on pin 16, output to channel 5
// const PinBlock_t gPinBlocks[]  = {
//     {DIODE_MATRIX,  ACTIVE_LOW, 16, 4, 20, 8, PEDAL32_MAX_NOTES, PEDAL_LOW_NOTENUM, 5}
// };

// A block of 32 parallel inputs starting on pin 16, output to channel 4
// const PinBlock_t gPinBlocks[]  = {
//     {PARALLEL, ACTIVE_LOW, LED_BUILTIN, 0,  16, 32,  32, 24, 4}
// };

// experimental test of struct modified for multi-contact systems
const PinBlockMulti_t gPinBlocksMulti[] = {
    {1, 8, 8, KEYBOARD61_MAX_NOTES, DIODE_MATRIX, ACTIVE_LOW, KEYBOARD61_LOW_NOTENUM, 5, { {16, 26}, {0,0}, {0,0} }},     // 8X8 single contact
    {2, 8, 8, KEYBOARD61_MAX_NOTES, DIODE_MATRIX, ACTIVE_LOW, KEYBOARD61_LOW_NOTENUM, 6, { {16, 26}, {32,40}, {0,0} }}    // 8x8 double contact
};
