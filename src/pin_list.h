#pragma once
#include <Arduino.h>
#include <MIDI.h>

#include "data.h"
#include "midi_instruments.h"
#include "debouncer.h"

// Pin block definition for both matrix and parallel scan organization
// All instances of this should be static (const).  Cannot be a class because of this.
// Relies on implicitly incrementing MIDI note numbers and debouncer index.
// Note numbers are consecutive for [selectPin, readPin] tuples where readPin varies fastest, offset from baseMidiNoteNum
// Debouncer index is also consecutive in the same order, offset from a separately stored list of baseDebouncerIndex per block.
//
// I rejected the idea of making the MIDI output interface configurable per block; instead making a design decision to
// always send outputs to the same interface.  This means no splitter / distribution capability, but that seems immaterial
// for the expected use cases of this code.
//
// TODO: Implmement hardcoded discontiguous pin blocks covering pins 2-3, 5-9, 11-12, 16-53 (cannot use 4, 10, 13, 14-15)
// TODO: Implement double-contact pin blocks.
//
typedef struct PinBlock {
    bool useSelect;                 // true if block is for a diode matrix using select pins
    bool activeLow;                 // true if input signals are active low
    int selectBasePin;              // start of select pin range, ignored if useSelect == false
    int numSelectPins;              // num of select pins, ignored if useSelect == false
    int readBasePin;                // start of read pin range
    int numReadPins;                // num of read pins
    int numCtrls;                   // total number of controls (e.g. notes) scanned by the block.  Typ. 61 for organ keyboards.
    midi::DataByte baseMidiNoteNum; // start of contiguous MIDI note num range
    midi::Channel midiOutChan;      // MIDI output channel for the block
} PinBlock_t;

// experimental struct with a union for multi-contact blocks

#define MAX_CONTACTS 3
typedef struct PbPinInfo{
    int selectBasePin;              // start of select pin range, ignored if useSelect == false
    int readBasePin;                // start of read pin range
} PbPinInfo_t;

typedef struct PinBlockMultContact {
    int numContacts;                // number of contacts per key: 1 = regular, 2 = velocity, 3 = aftertouch
    int numSelectPins;              // num of select pins, ignored if useSelect == false
    int numReadPins;                // num of read pins
    int numCtrls;                   // number of notes must be the same in all sub-blocks
    bool useSelect;                 // all sub-blocks in the group must have same layout (parallel or matrix)
    bool activeLow;                 // all sub-blocks must have same input polarity
    midi::DataByte baseMidiNoteNum; // start of contiguous MIDI note num range
    midi::Channel midiOutChan;      // MIDI output channel for the block
    PbPinInfo_t pbPinInfo[MAX_CONTACTS];
} PinBlockMulti_t;



// Include specific configuration here - it requires the above typedef
#include "config_scan_pins.h"


const int nPinBlocks = sizeof(gPinBlocks) / sizeof(PinBlock_t);

int calcPinBlockSize(int pbIndx);
int calcDebouncerBase(int pbIndx);
void initDebouncerBases();
void initDebouncers();
int calcDebouncerIndx(int pbIndx, int selectPin, int readPin);
int getPinBlockIndxFromDebouncerIndx( int debIndx );






// *** OBSOLETE BUT KEPT FOR REFERENCE TEMPORARILY ***
#if 0
typedef struct MatrixEnt_Single_Dynamic {
    DebouncerMidiNoteSingleContact debouncer;          // less RAM to directly include 20-byte debouncer object - need to get it constructed right
//  midi::DataByte midiNoteNum;                        // needs to be dynamic param only if expanding a PinBlock spec
//  MatrixEnt_Single_Static_t * pMatrixEnt;            // pointer to matching static scanlist entry in flash - do we need this back index?
    
    // *** don't duplicate static params in RAM, maintain parallel list of objects with notenum and single-contact debouncers
    // at 32 bytes each four 8x8's (256 entries) would be ~8K, so gotta be somewhat smaller
    // bool useSelect;
    // int selectPin;
    // int readPin;
    // midi::Channel midiOutChan;
    
} MatrixEnt_Single_Dynamic_t;


// Master pin list used by the scanner for all single-contact triggers
class ScanList_Single_Dyn {
    protected:
        midi::DataByte nextMidiNote = 20;                   // next midi note num to be assigned
        unsigned long debounceMsec = 20;                    // ?? do we do this on a per-output basis?
        t_midiInterfaceHWSerialPtr midiInterface;
        // The next list must be held in parallel with the static scanlist that is used to construct us
        //List<MatrixEnt_Single_Dynamic_t> scanList_dyn;

    public:
        void addScanList(scanlist_static_t scanlist) {
            // We don't assign the running note num here - specified in the incoming list; we just track the highest one seen
            midi::DataByte highestNote = nextMidiNote - 1;
            for (unsigned int i = 0; i < scanlist.Count(); i++) {
                MatrixEnt_Single_Dynamic_t newEnt;
                newEnt.pMatrixEnt = &(scanlist[i]);         // record index of flash scanlist record (if we can use an index it should save 2 bytes)
                newEnt.useSelect = scanlist[i].useSelect;
                newEnt.selectPin = scanlist[i].selectPin;
                newEnt.readPin = scanlist[i].readPin;
                newEnt.midiNoteNum = scanlist[i].midiNoteNum;
                highestNote = (newEnt.midiNoteNum > highestNote) ? newEnt.midiNoteNum : highestNote;
                newEnt.midiOutChan = scanlist[i].midiOutChan;
                newEnt.debouncer = new DebouncerMidiNoteSingleContact(debounceMsec, midiInterface, newEnt.midiNoteNum, newEnt.midiOutChan);
                scanList_dyn.Add(newEnt);
            }
            nextMidiNote = highestNote + 1;
        }

        // expand a matrix pin block def and add the resulting entries to the scan list
        // >>>This would require the full dynamic scan list, cannot support 4 8x8's this way, won't fit in 8K RAM
        void addPinBlock(PinBlock_t pinblock) {
            for (int i = pinblock.selectBasePin; i < pinblock.selectBasePin + pinblock.numSelectPins; i++) {
                for (int j = pinblock.readBasePin; j < pinblock.readBasePin + pinblock.numReadPins; j++) {
                    MatrixEnt_Single_Dynamic_t newEnt;
                    newEnt.useSelect = pinblock.useSelect;
                    newEnt.selectPin = i;
                    newEnt.readPin = j;
                    newEnt.midiNoteNum = nextMidiNote++;
                    newEnt.midiOutChan = pinblock.midiOutChan;
                    newEnt.debouncer = new DebouncerMidiNoteSingleContact(debounceMsec, midiInterface, newEnt.midiNoteNum, newEnt.midiOutChan);
                    scanList_dyn.Add(newEnt);
                }
            }
        }

};
#endif
