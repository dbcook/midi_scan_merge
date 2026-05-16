#pragma once

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
// It's possible to increase capacity by having multiple diode matrix blocks share the same
// scan pins as long as everything has the same polarity.  This enables a theoretical maximum
// of seven 8x8 diode matrix blocks on one Arduino Due.
//
// CAVEAT: there are various logical constraints on the pinBlock definitions that are not
// enforced in the code (yet).  These include:
//      The scan pins range and read pins range for a diode matrix must not overlap
//      (base note number + number of notes - 1) must not exceed 127
//      No pin range can extend beyond pin 69 (Mega 2560 and Due)
//
// Discontiguous pin ranges for a diode matrix are not yet supported.  However you can map
// arbitrary pins as parallel inputs just by creating multiple parallel blocks that are
// individually contiguous.
//
// For each group, in addition to the Arduino pin ranges you get to specify:
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
//    D0-D1 - hardware serial port reserved for bootloader serial, debugger stub, and console debug messages
//    D4  - Ethernet Shield 2: chip select for SD card
//    D10 - Ethernet Shield 2: chip select for WizNet 5500 chip
//    D13 - LED output on most Arduino implementations
//    D14-D15 - Unavailable if you have a MIDI serial shield connected to SER3 
//
// If you don't have a MIDI serial shield you can freely use all pins from D14 to D53 as well as all 16 analog pins,
// giving 56 readily available inputs, plus 3 short sequences (D2-3, D5-9, D11-12) that could be defined as
// parallel blocks.  That will allow at least three 8x8 diode matrix keyboards (up to six with shared scan pin ranges
// on a Due/Teensy) plus 9 discretes (pistons etc.).
//
// A few constants to make the block defs more readable
#define DIODE_MATRIX true
#define PARALLEL false
#define ACTIVE_LOW true

// A single 8x8 keyboard matrix, 61 notes, starting on pin 16, output to channel 8
// const PinBlock_t gPinBlocks[]  = {
//     {DIODE_MATRIX,  ACTIVE_LOW, 16, 8, 24, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 8, 20, 5}
// };

// This example has the maximum number of 8x8 matrix blocks that each processor can handle
// A Due can scan four at 0.9 KHz and seven at 540 Hz which is perfectly good
// The Mega 2560 can do 3 at 780 Hz, but at 4 8x8's there is an obvious stack/heap collision with 256 debouncers leaving ~1900 bytes free memory
// Therefore the Mega build has been set to only allocate 192 debouncers (3 keyboards' worth)
#if 0
const PinBlock_t gPinBlocks[]  = {
#if defined(__SAM3X8E__)
    {DIODE_MATRIX,  ACTIVE_LOW, 16, 8, 24, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 5, 20, 5}
   ,{DIODE_MATRIX,  ACTIVE_LOW, 16, 8, 32, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 6, 20, 5}
   ,{DIODE_MATRIX,  ACTIVE_LOW, 16, 8, 40, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 7, 20, 5}
   ,{DIODE_MATRIX,  ACTIVE_LOW, 16, 8, 48, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 8, 20, 5}
   ,{DIODE_MATRIX,  ACTIVE_LOW, 16, 8, 48, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 9, 20, 5}
   ,{DIODE_MATRIX,  ACTIVE_LOW, 16, 8, 48, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 10, 20, 5}
   ,{DIODE_MATRIX,  ACTIVE_LOW, 16, 8, 48, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 11, 20, 5}
#elif defined(ARDUINO_AVR_MEGA2560)
    {DIODE_MATRIX,  ACTIVE_LOW, 16, 8, 24, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 5, 20, 5}
   ,{DIODE_MATRIX,  ACTIVE_LOW, 16, 8, 32, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 6, 20, 5}
   ,{DIODE_MATRIX,  ACTIVE_LOW, 16, 8, 40, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 7, 20, 5}
#else
#error "Unsupported processor type!"
#endif

};
#endif

// 4x8 pedalboard, 32 notes, starting on pin 16, output to channel 5
// const PinBlock_t gPinBlocks[]  = {
//     {DIODE_MATRIX,  ACTIVE_LOW, 16, 4, 20, 8, PEDAL32_MAX_NOTES, PEDAL_LOW_NOTENUM, 5, 20, 5}
// };

// A block of 32 parallel inputs starting on pin 16, output to channel 4
const PinBlock_t gPinBlocks[]  = {
    {PARALLEL, ACTIVE_LOW, LED_BUILTIN, 0,  16, 32,  32, 24, 4, 20, 5}
};
// experimental test of struct modified for multi-contact systems
const PinBlockMulti_t gPinBlocksMulti[] = {
//    {DIODE_MATRIX, ACTIVE_LOW, 1, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 5, 20, 5, { {16, 26}, {0,0}, {0,0} }},     // 8X8 single contact
//    {DIODE_MATRIX, ACTIVE_LOW, 2, 8, 8, KEYBOARD61_MAX_NOTES, KEYBOARD61_LOW_NOTENUM, 6, 20, 5, { {16, 26}, {32,40}, {0,0} }}    // 8x8 double contact
};
