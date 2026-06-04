#pragma once

#include <Arduino.h>
#include "glob_gen.h"
#include "fastread.h"
#include "data.h"
#include "spindie.h"

// Digital input pin scan block processor
// This class also incorporates digital/analog conflict detection
//
// We assume here that pin numbers cannot exceed 255.  The largest Arduino style module seen so far has 70.
//
// It might be structurally nice to have a base class with the core pinBlock based scan logic and a virtual
// method for what to do for each input.  However, the function call involved causes a noticeable reduction
// in scan rate.
class InputScanner {
    public:

    // validate and configure all digital scan and read pins in accordance with the pinBlock definitions
    // When doConfig is false, we only test for illegal pins
    static void configureDigitaPins(bool doConfig = false) {
        for (auto pbi = gPinBlocksDigital.begin(); pbi != gPinBlocksDigital.end(); pbi++) {
            int numReadPIns =  (pbi->numCtrls < pbi->numReadPins) ? pbi->numCtrls : pbi->numReadPins;
            int numSelPins = pbi->numSelectPins;
            for (auto contactIndx = 0; contactIndx < pbi->numContacts; contactIndx++) {
                int readBase = pbi->pbPinInfo[contactIndx].readBasePin;
                if (pbi->useSelect) {
                    int selBase = pbi->pbPinInfo[contactIndx].selectBasePin;
                    for (auto j = selBase; j < selBase + numSelPins; j++) {
                        PinList::checkLegalPin(j, "Unusable Sel Pin");
                        if (doConfig) {
                            // scan pins get configured as output
                            pinMode(j, OUTPUT);
                            fastwrite(j, pbi->activeLow ? HIGH : LOW);
                            AM_DBG(F("Pin"), j, F("Output"));
                        }
                    }
                }
                for (auto j = readBase; j < readBase + numReadPIns; j++ ) {
                    PinList::checkLegalPin(j, "Unusuable Read Pin");
                    if (doConfig) {
                        // read pins get configured as INPUT_PULLUP
                        pinMode(j, pbi->activeLow ? INPUT_PULLUP : INPUT);
                        AM_DBG(F("Pin"), j, F("Pullup"));
                    }
                }
            } // contacts loop
        } // pinBlock loop
    }

    // configure analog input pin blocks - set as input with pullup disabled
    // The analog pins merely need to not be in output mode - analogRead and digitalRead will both work on an analog pin in input mode.
    // We disable the pullup here in case somehow we enter with the pullup enabled.
    // analogRead will still work if the pullup is on but the readings will be inaccurate
    static void configureAnalogPins(bool doConfig = false) {
        for (auto pbi = gPinBlocksAnalog.begin(); pbi != gPinBlocksAnalog.end(); pbi++) {
            for (int anPin = pbi->basePin; anPin < pbi->basePin + pbi->numPins; anPin++) {
                // check for patently illegal pin - on GCM4 there are some in the midst of the analog pin range
                PinList::checkLegalPin(anPin, "Unusable AnalogIn Pin");
                // make sure tha analog pin is actually an analog capable pin
                PinList::checkLegalAnalogPin(anPin, "Pin Not AnalogIn");
                if (doConfig) {
                    pinMode(anPin, INPUT);
                    AM_DBG(F("Pin"), anPin, F("AnInput"));
                }
            }
        }
    }

    // Scan all in-memory digital pinBlocks
    // NOTE: fastread / digitalWriteFast not implemented on Grand Central SAMD51, falls back to digitalRead and digitalWrite
    //       not a problem because the processor is quite fast.  fastread is important on the much slower Due.
    // There is a hardware based performance subtlety here.
    // We *MUST* insert a delay after writing the select pin to allow the read pins to swing.
    // RC time constant is 6-7 usec (220 ohm pullup and 30 pf parasitic capacitance) so we need ~8-10 usec
    // We could restore most of the scan frequency (but not the latency) by queing the scan states and then
    // calling a function here to process them on the next pass.
    // But given that a 1 KHz sample scan rate is more than enough, we don't need to add the extra complexity now.
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

                        fastwrite(selPin, pb->activeLow ? LOW : HIGH);
                        delayMicroseconds(gConfig.matrixStabilizationUsec);

                        for (auto readPin = pbi->readBasePin; (readPin < rlim) && (noteNum < noteLim); readPin++) {
                            if (gConfig.logScanSequence) {
                                AM_DBG(F("Ch"), pb->midiOutChan, F("Nt"), noteNum, F("Sel"), selPin, F("Rd"), readPin, F("Db"), dbIndx);
                            }
                            int inp = fastread(readPin);
                            if (gConfig.logScanSequence) {
                                AM_DBG(F("pv"), readPin, inp);
                            }
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
                        fastwrite(selPin, pb->activeLow ? HIGH : LOW);
                    }
                }
            }
            else {
                // parallel - single contact only.  Not enough pins to do dual contact except on pedalboard with USB transport ONLY.
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

    // scans a lot of pins with fastread but takes no action - reads are safe even if pin is configured as output
    // thus requires no setup, but you ought not hit the forbidden pins 62-66 on the Grand Central
    // nor the SPI on 50-52 if Ethernet is active.  So we stop at 49.
    static void test_fastread() {
        for (uint8_t i = 2; i < 49; i++) {
            fastread(i);
        }
    }

    // simulated 8x8 diode matrix that only reads the pin. Select base = 22, read base = 30
    // *** to use, must separately config the pins - currently no code for that
    static void test_diodeMatrix_8x8(byte *buf) {
        for (int colPin = 22; colPin < 30; colPin++) {
            fastwrite(colPin, LOW);
            int indx = 0;
            for (int readPin = 30; readPin < 38; readPin++) {
                buf[indx++] = fastread(readPin);
            }
            fastwrite(colPin, HIGH);
        }
    }

    // check for select/read and digital/analog conflicts
    //    no select pin range may intersect any read pin range
    //    no digital pin range may intersect any analog pin range
    static void checkPinFunctionConflicts() {
        for (auto pbi = gPinBlocksDigital.begin(); pbi != gPinBlocksDigital.end(); pbi++) {
            int numReadPIns =  (pbi->numCtrls < pbi->numReadPins) ? pbi->numCtrls : pbi->numReadPins;
            int numSelPins = pbi->numSelectPins;

            // make sure notenum doesn't go out of range even if pin numbers are OK
            int baseNote = pbi->baseMidiNoteNum;
            if ((baseNote + (int)(pbi->numCtrls)) > MIDI_MAX_NOTES)
                _SpinDie("NoteNum too big", baseNote + pbi->numCtrls - 1);

            for (auto contactIndx = 0; contactIndx < pbi->numContacts; contactIndx++) {
                int readBase = pbi->pbPinInfo[contactIndx].readBasePin;
                if (pbi->useSelect) {
                    int selBase = pbi->pbPinInfo[contactIndx].selectBasePin;
                    for (auto sPin = selBase; sPin < selBase + numSelPins; sPin++) {
                        // is this select pin in any read pin range (matrix or parallel) in any pinBlock (including this one)?
                        // all digital input pinBlocks have a read pin range
                        for (auto pbix = gPinBlocksDigital.begin(); pbix != gPinBlocksDigital.end(); pbix++) {
                            int nrpins = (pbix->numCtrls < pbix->numReadPins) ? pbix->numCtrls : pbix->numReadPins;
                            for (auto cti = 0; cti < pbix->numContacts; cti++) {
                                int rpbase = pbix->pbPinInfo->readBasePin;
                                for (int n = rpbase; n < rpbase + nrpins; n++) {
                                    if (sPin == n) _SpinDie("Sel/Rd Conflict", sPin);
                                }
                            } // inner contacts loop
                        } // inner digital pinBlock loop

                        // check this select pin against all analog pin blocks
                        for (auto pbia = gPinBlocksAnalog.begin(); pbia != gPinBlocksAnalog.end(); pbia++) {
                            int napins = pbia->numPins;
                            for (int apin = 0; apin < napins; apin++) {
                                if (sPin == apin) _SpinDie("Dig/An Conflict", sPin);
                            }
                        } // analog pinBlock loop - inner
                    }

                    // nothing else to check for read pins vs other matrix digital read pins since select lines can be shared
                    // parallel block read pins are checked below
                    // there are some weird cases with shared select lines with differing read pin ranges, but I think
                    // that as long as the select and read ranges don't touch each other that everything is permitted

                    // check matrix read pins against all analog pin blocks
                    for (auto pin = readBase; pin < readBase + numReadPIns; pin++ ) {
                        for (auto pbix = gPinBlocksAnalog.begin(); pbix != gPinBlocksAnalog.end(); pbix++) {
                            int napins = pbix->numPins;
                            for (int apin = pbix->basePin; apin < pbix->basePin + napins; apin++) {
                                if (pin == apin) _SpinDie("Dig/An Conflict", pin);
                            }
                        }
                    }

                } // matrix
                else {
                    // parallel block read pins cannot appear in any other read pin range *except this one*, whether matrix or parallel
                    for (auto pin = readBase; pin < readBase + numReadPIns; pin++ ) {
                        for (auto pbix = gPinBlocksDigital.begin(); pbix != gPinBlocksDigital.end(); pbix++) {
                            if (pbix == pbi) continue; // avoid self-conflict
                            int nrpins = (pbix->numCtrls < pbix->numReadPins) ? pbix->numCtrls : pbix->numReadPins;
                            for (auto cti = 0; cti < pbix->numContacts; cti++) {
                                int rpbase = pbix->pbPinInfo->readBasePin;
                                for (int n = rpbase; n < rpbase + nrpins; n++) {
                                    if (pin == n) _SpinDie("ParrRd Conflict", pin);
                                } // read pin loop - inner
                            } // contacts loop - inner
                        } // digital pinBlock loop - inner

                        // check this read pin against all analog pin blocks
                        for (auto pbia = gPinBlocksAnalog.begin(); pbia != gPinBlocksAnalog.end(); pbia++) {
                            int napins = pbia->numPins;
                            for (int apin = pbia->basePin; apin < pbia->basePin + napins; apin++) {
                                if (pin == apin) _SpinDie("Dig/An Conflict", pin);
                            }
                        } // analog pinBlock loop - inner

                    } // read pin loop - outer
                }

                // check for digital / analog collisions

            } // contacts loop - outer
        } // pinBlock loop - outer
    }

};
