#pragma once
#include <Arduino.h>
#include <MIDI.h>
//#include <ListLib.h>
//#include "ArrayList.h"
#include "data.h"
#include "debouncer.h"

// Pin block definition for both matrix and parallel scan organization
// All instances of this should be static
// Note numbers are consecutive for [selectPin, readPin] tuples where readPin varies fastest, offset from baseMidiNoteNum
// Debouncer index is also consecutive in the same order, offset from baseDebouncerIndex
// if useSelect is false, selectBasePin and numSelectPins are ignored
// Can't be a class since we have to define these in flash via
// const PinBlock_t gPinBlocks[] = {
//     {false, 0, 13, 0, 8, 0, 20, 4},
//     {true, 28, 8, 36, 8, 0, 20, 5}
// };
// const int nPinBlocks = sizeof(gPinBlocks) / sizeof(PinBlock_t);
//
typedef struct PinBlock {
    bool useSelect;                 // true if block is for a diode matrix using select pins
    int selectBasePin;              // start of select pin range, ignored if useSelect == false
    int numSelectPins;              // num of select pins, ignored if useSelect == false
    int readBasePin;                // start of read pin range
    int numReadPins;                // num of read pins
    midi::DataByte baseMidiNoteNum; // starting of contiguous MIDI note num range
    midi::Channel midiOutChan;      // MIDI output channel for the block
} PinBlock_t;

// EXAMPLE PIN / NOTE CONFIG
//  first block is 8 parallel pins starting at pin 20 and note num 32, output to MIDI chan 4
//  second block is an 8x6 diode matrix with select pins starting at 28, read pins at 36, note nums starting at 20, sent to chan 5
// Note that when useSelect is false, you should set the select pin to 13 (the LED) so that even if some bug
// causes the select to be written, it will just blink the LED :
const PinBlock_t gPinBlocks[] = {
    {false, 13, 0, 20, 8, 32, 4},
    {true, 28, 8, 36, 6, 20, 5}
};
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
