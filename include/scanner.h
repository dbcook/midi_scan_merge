#pragma once

#include <Arduino.h>
#include "glob_gen.h"
#include "fastread.h"
#include "data.h"

// Minimal-RAM scan block processor based on flash PinBlock defs and separately allocated debouncers.
//
// We assume here that pin numbers cannot exceed 255.  The largest module seen so far has 70.
// It would be nice to have a base class with the core pinBlock based scan logic and a virtual
// method for what to do for each input.  However, the function call involved causes a
// noticeable reduction in scan rate.
class InputScanner {
    public:

    // scans all in-memory digital pinBlocks
    static void scanDigitalPinBlocks() {
        for (size_t i = 0; i < gPinBlocksDigital.size(); i++) {
            const PinBlockMulti_t * pb = &(gPinBlocksDigital[i]);
            midi::DataByte noteNum = pb->baseMidiNoteNum;
            midi::DataByte noteLim = pb->baseMidiNoteNum + pb->numCtrls;        // in case matrix has a non-full select row
            int dbIndx = gDebouncerBases[i];
            if (gConfig.logScanSequence) {
            }
            if (pb->useSelect) {
                // matrix, 1-3 contacts
                if (pb->numContacts == 1) {
                    const PbPinInfo_t * pbi = &(pb->pbPinInfo[0]);              // contains base select and base read pin
                    uint8_t clim = pbi->selectBasePin + pb->numSelectPins;
                    uint8_t rlim = pbi->readBasePin + pb->numReadPins;
                    for (auto selPin = pbi->selectBasePin; selPin < clim; selPin++) {

                        // We *MUST* insert a delay after the fastwrite to allow the read pins to swing.  RC time constant is 6-7 usec so we need ~8-10 usec
                        // 10 usec delay puts the maximum capacity 7-keyboard stress test scan (427 inputs) at 1.0 KHz
                        // We could restore most of the scan frequency (but not the 0.5 msec latency) by queing the scan states and then
                        // calling a function here to process them on the next pass.
                        // But given that 1 KHz sample scan rate is more than enough, we don't need to add the extra complexity.
                        fastwrite(selPin, pb->activeLow ? LOW : HIGH);
                        delayMicroseconds(gConfig.matrixStabilizationUsec);

                        for (auto readPin = pbi->readBasePin; (readPin < rlim) && (noteNum < noteLim); readPin++) {
                            if (gConfig.logScanSequence) {
                                AM_DBG(F("Ch"), pb->midiOutChan, F("Nt"), noteNum, F("Sel"), selPin, F("Rd"), readPin, F("Db"), dbIndx);
                            }
                            int inp = fastread(readPin);
                            bool istate = (pb->activeLow) ? !inp : inp;
                            if (istate) {
                                gDebouncers[dbIndx].stateSampleActive();
                            }
                            else {
                                gDebouncers[dbIndx].stateSampleInactive();
                            }
                            // Alternate per-read-pin handlers that were useful for testing
                            //gDebouncers[dbIndx].stateSampleActive();    // causes initial burst of noteOn for all notes

                            dbIndx++;
                            noteNum++;
                        }
                    }
                }
            }
            else {
                // parallel - single contact only?
                if (pb->numContacts == 1) {
                    const PbPinInfo_t * pbi = &(pb->pbPinInfo[0]);              // contains base select and base read pin
                    uint8_t rlim = pbi->readBasePin + pb->numReadPins;
                    for (auto readPin = pbi->readBasePin; (readPin < rlim) && (noteNum < noteLim); readPin++) {
                        int inp = fastread(readPin);
                        bool istate = (pb->activeLow) ? !inp : inp;
                        if (istate) {
                            gDebouncers[dbIndx].stateSampleActive();
                        }
                        else {
                            gDebouncers[dbIndx].stateSampleInactive();
                        }
                        dbIndx++;
                        noteNum++;
                    }
                }
            }

        }

    }

    static void scanAnalogPinBlocks() {
        for (uint16_t i; i < gPinBlocksAnalog.size(); i++) {

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
