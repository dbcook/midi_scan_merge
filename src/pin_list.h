#pragma once
#include <Arduino.h>
#include <MIDI.h>

#include "data.h"
#include "midi_instruments.h"
#include "debouncer.h"
#include "ArrayList.h"

// Pin block definition for both matrix and parallel scan organization
// All instances of this should be static (const).  Cannot be a class because of this.
// Relies on implicitly incrementing MIDI note numbers and debouncer index.
// Note numbers are consecutive for [selectPin, readPin] tuples where readPin varies fastest, offset from baseMidiNoteNum
// Debouncer index is also consecutive in the same order, offset from a separately stored list of baseDebouncerIndex per block.
//
// I rejected the idea of making the MIDI output interface configurable per block; instead making a design decision to
// always send outputs to the same interface.  This means no splitter / distribution capability, but that seems immaterial
// for the expected use cases of this code.

// struct with an embedded union for multi-contact blocks

#define MAX_CONTACTS 3

typedef struct PbPinInfo{
    int selectBasePin;              // start of select pin range, ignored if useSelect == false
    int readBasePin;                // start of read pin range
} PbPinInfo_t;

typedef struct PinBlockMultContact {
    bool useSelect;                 // all sub-blocks in the group must have same layout (parallel or matrix)
    bool activeLow;                 // all sub-blocks must have same input polarity
    int numContacts;                // number of contacts per key: 1 = regular, 2 = velocity, 3 = aftertouch
    int numSelectPins;              // num of select pins, ignored if useSelect == false
    int numReadPins;                // num of read pins
    int numCtrls;                   // number of notes must be the same in all sub-blocks
    midi::DataByte baseMidiNoteNum; // start of contiguous MIDI note num range
    midi::Channel midiOutChan;      // MIDI output channel for the block
    uint8_t attackDebounceMsec;     // debounce time for attack
    uint8_t releaseDebounceMsec;    // debounce time for release
    PbPinInfo_t pbPinInfo[MAX_CONTACTS];
} PinBlockMulti_t;

// struct for analog input blocks with consecutive CC numbers and individually tunable low-pass filtering.
// Resolution: we always tell the Arduino runtimes to operate as if we had 16-bit resolution.
// There is no harm in that even though older boards have 10 or 12 bit resolution; the LSBs in the 16-bit result are just zero.
// Filtering is done via the one-line classic exponential smoothing filter
//   val = (alpha * sample) + (1 - alpha) * val
// where alpha [0,1) is the sensitivity constant
// There is no center position parameter since auto centering joysticks are rare within the instrument use case
// Everything is done in floating point since the Grand Central has an FPU
typedef struct PinBlockAnalogRead {
    uint8_t basePin;                // starting pin number for this block (easiest to use A0, A1, etc.)
    uint8_t numPins;                // number of pins in this block (consecutive CC numbers)
    midi::DataByte baseCCNum;       // starting CC number for inputs in this block
    float deadband;                 // center deadband in percent of range
    float lowEndband;               // guardband at bottom in percent of range
    float highEndBand;              // guardband at top in percent of range
    float filterAlpha;              // constant for lowpass filter, typ. about 0.1 to 0.3
} PinBlockAnalogRead_t;


// Include specific configuration
#include "config_scan_pins.h"

// We can take compile time sizes of the hardcoded flash pinblock groups
const int nFlashPinBlocksMulti = sizeof(gFlashPinBlocksMulti) / sizeof(PinBlockMulti_t);
const int nFlashPinBlocksAnalogRead = sizeof(gFlashPinBlocksAnalogRead) / sizeof(PinBlockAnalogRead_t);

// declare the RAM based pinblock lists
EXTERN ArrayList<PinBlockMulti_t> gPinBlocksDigital;
EXTERN ArrayList<PinBlockAnalogRead_t> gPinBlocksAnalog;

void initMemPinBlocks();
int calcPinBlockSize(int pbIndx);
int calcDebouncerBase(int pbIndx);
void initDebouncerBases();
void initDebouncers();
int calcNumDigitalInputs();
int calcNumAnalogInputs();
int calcDebouncerIndx(int pbIndx, int selectPin, int readPin);
