#pragma once

//-----------------------------------------------------------------------
// Configures the scanning of input pins
//-----------------------------------------------------------------------
// Here is where you specify the diode matrix and parallel input pins layout for the scanner.
//
// For each group, in addition to the Arduino input pin ranges you get to specify:
//   The total number of notes to scan (for diode matrix where you likely will use 61 notes out of an 8x8 matrix)
//   The base MIDI note number for the group
//   The MIDI output channel for the NoteOn and NoteOff messages.  
//      If the note ranges for 2 groups overlap, then each group needs to be sent to a different channel.
//
// This must define a const and fully initialized array of PinBlock_t named "gPinBlocks". 
// To see the PinBlock_t struct declaration, right-click PinBlock_t below and use "go to definition"
// In the example shown below:
//     The first block is 8 parallel pins starting at pin 20 and note num 32, output to MIDI chan 4
//     The second block is an 8x6 diode matrix with select pins starting at 28, read pins at 36, note nums starting at 20, sent to chan 5
//
// When useSelect is false, you should set the select pin to 13 (the LED) so that even if some bug
// causes the select to be written, it will just blink the LED.
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

const PinBlock_t gPinBlocks[]  = {
    {false, 13, 0,  5, 5,  5, 32, 4},         // 5 parallel inputs starting at pin 5
    {true,  16, 8, 24, 8, 61, 20, 5}         // a full 61 note keyboard on a 8x8 matrix starting at pin 16
};
