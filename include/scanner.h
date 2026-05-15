#pragma once

#include <Arduino.h>
#include "glob_gen.h"
#include "data.h"

// Minimal-RAM scan block processor based on flash PinBlock defs and separately allocated debouncers.
// This design allows the slow Mega 2560 with its 8KB of RAMto be used effectively
//
// It would be nice to have a base class with the core pinBlock based scan logic and a virtual
// method for what to do for each input.  However, the function call involved causes a
// noticeable reduction in scan rate.
class InputScanner {
    public:
    static void scanPinBlocksSingleContact() {
        for (int i = 0; i < nPinBlocks; i++) {
            // copying the PinBlock struct into RAM is almost twice as slow as reading directly from flash; don't do it
            const PinBlock_t * pb = &(gPinBlocks[i]);
            midi::DataByte noteNum = pb->baseMidiNoteNum;
            midi::DataByte noteLim = pb->baseMidiNoteNum + pb->numCtrls;        // in case matrix has a non-full select row
            int dbIndx = gDebouncerBases[i];

            // These msgs help confirm that the PinBlock is being read and processed correctly
    #if 0
            Console_print("cbase "); Console_println(pb->selectBasePin);
            Console_print("clim "); Console_println(pb->selectBasePin + pb->numSelectPins);
            Console_print("rbase "); Console_println(pb->readBasePin);
            Console_print("rlim "); Console_println(pb->readBasePin + pb->numReadPins);
    #endif
            // process the pin block
            if (pb->useSelect) {
                // diode matrix - read pins loop inside of select pins loop
                int clim = pb->selectBasePin + pb->numSelectPins;
                int rlim = pb->readBasePin + pb->numReadPins;
                for (int selPin = pb->selectBasePin; selPin < clim; selPin++) {
                    fastwrite(selPin, pb->activeLow ? LOW : HIGH);

                    // scan the read pins for this select pin
                    for (int readPin = pb->readBasePin; (readPin < rlim) && (noteNum < noteLim); readPin++) {
                        uint8_t inp = fastread(readPin);

#if LOG_SCAN_SEQUENCE
                        // alt scan action to log MIDI channel, noteNum, selectPin, readPin, dbIndx
                        // if this is enabled, there must be external logic to only call this only once every 10 sec or so due to large serial output
                        AM_DBG(F("Ch"), pb->midiOutChan, F("Nt"), noteNum, F("Sel"), selPin, F("Rd"), readPin, F("Db"), dbIndx);
#else
                        // machinations to eliminate args to the debouncer - gave a considerable speedup
                        // this straightforward code was slower:
                        // gDebouncers[dbIndx++].stateSample(pb->activeLow ? !inp : inp, noteNum++, pb->midiOutChan);
                        bool istate = (pb->activeLow)? !inp : inp;
                        if (istate) {
                            gDebouncers[dbIndx].stateSampleActive();
                        }
                        else {
                            gDebouncers[dbIndx].stateSampleInactive();
                        }
#endif
                        // Alternate per-read-pin handlers that were useful for testing
                        //gDebouncers[dbIndx].stateSampleDummy(pb->activeLow ? !inp : inp); // logs notes and chans sequence but not pins
                        //gDebouncers[dbIndx].stateSampleActive();    // causes initial burst of noteOn for all notes

                        dbIndx++;
                        noteNum++;
                    }
                    fastwrite(selPin, pb->activeLow ? HIGH : LOW);  // deactivate the select pin
                }
            }
            else {
                // parallel non-matrix inputs - single loop on read pins
                int rlim = pb->readBasePin + pb->numReadPins;
                for (int readPin = pb->readBasePin; readPin < rlim; readPin++) {
                    int inp = fastread(readPin);
#if LOG_SCAN_SEQUENCE
                        // alt scan action to log MIDI channel, noteNum, selectPin, readPin, dbIndx
                        AM_DBG(F("Ch"), pb->midiOutChan, F("Nt"), noteNum, F("Rd"), readPin, F("Db"), dbIndx);
#else
                        gDebouncers[dbIndx++].stateSample(pb->activeLow ? !inp : inp, noteNum++, pb->midiOutChan);
#endif
                }
            }

        }
    }

    // scans all pins with fastread but takes no action - reads are safe even if pin is configured as output
    // thus requires no setup
    static void test_fastread() {
        for (uint8_t i = 2; i < 69; i++) {
            fastread(i);
        }
    }

    // simulated 8x8 diode matrix - *** must separately config the scan pins as output
    static void test_diodeMatrix_8x8(byte *buf) {
        for (int colPin = 20; colPin < 28; colPin++) {
            fastwrite(colPin, LOW);
            int indx = 0;
            for (int readPin = 28; readPin < 36; readPin++) {
                buf[indx++] = fastread(readPin);
            }
            fastwrite(colPin, HIGH);
        }
    }

};
