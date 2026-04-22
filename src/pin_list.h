#pragma once
#include <Arduino.h>
#include <MIDI.h>
#include <ListLib.h>
//#include "ArrayList.h"
#include "debouncer.h"


// A list of these will map each matrix single-contact input [select, read] with its midi note and output channel
// The static version of this is designed to be instantiable as a const struct array in flash.

// The dynamic version is built in RAM and can be populated in various ways:
//    Reading one or more instances of the static list
//    Loading from a pin block spec where the pin segments are contiguous and notes follow [select][read] order
//    Loading from a parallel block spec where there are no select pins specified (useSelect false everywhere)
//
// When defining a static array manually (e.g. if pins or notes jump around), BE SURE TO GROUP ON SELECT PIN NUMBER.
// This will allow scanner to work more efficiently without writing a different select pin for every read pin scanned.
//
// Defining an arbitrary scan list in flash:
// Note that when useSelect is false, you should set the select pin to 13 (the LED) so that even if some bug
// causes the select to be written, it will just blink the LED :)
//   const MatrixEnt_Single_Static_t myscan[] = {
//      { true, 10, 20, 32, 4 }
//     ,{ true, 10, 21, 33, 4 }
//     ,{ false,13, 22, 34, 4 }
//   }

typedef struct MatrixEnt_Single_Static {
    bool useSelect;
    int selectPin;
    int readPin;
    midi::DataByte midiNoteNum;
    midi::Channel midiOutChan;
} MatrixEnt_Single_Static_t;

typedef List<MatrixEnt_Single_Static> scanlist_static_t;


// Pin block definition for both matrix and parallel scan organization
// All instances of this should be static
// convention: note numbers auto-generated for [selectPin, readPin] tuples where readPin varies fastest
// if useSelect is false, selectBasePin and numSelectPins are ignored
typedef struct PinBlock {
    bool useSelect;             // applies to whole block
    int selectBasePin;
    int numSelectPins;
    int readBasePin;
    int numReadPins;
    midi::DataByte baseMidiNoteNum;
    midi::Channel midiOutChan;  // applies to whole block

} PinBlock_t;


typedef struct {
    midi::DataByte midiNoteNum;                        // needs to be dynamic param
    DebouncerMidiNoteSingleContact *debouncer;         // ? less RAM to directly include 20-byte debouncer object?  need to get it constructed right
} DebouncerNoteEnt_t;


typedef struct MatrixEnt_Single_Dynamic {
    DebouncerMidiNoteSingleContact *debouncer;         // ? less RAM to directly include 20-byte debouncer object?  need to get it constructed right
//  midi::DataByte midiNoteNum;                        // needs to be dynamic param only if expanding a PinBlock spec
    MatrixEnt_Single_Static_t * pMatrixEnt;            // pointer to matching static scanlist entry in flash
    
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
        List<MatrixEnt_Single_Dynamic_t> scanList_dyn;

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
