#pragma once

#include <Arduino.h>
#include <variant.h>
#include "glob_gen.h"
#include "fastread.h"
#include "data.h"
#include "spindie.h"

// Minimal-RAM scan block processor based on flash PinBlock defs and separately allocated debouncers.
//
// We assume here that pin numbers cannot exceed 255.  The largest module seen so far has 70.
// It would be nice to have a base class with the core pinBlock based scan logic and a virtual
// method for what to do for each input.  However, the function call involved causes a
// noticeable reduction in scan rate.
class InputScanner {
    public:

    // configure all digital scan and read pins in accordance with the pinBlock definitions
    // TODO enforce pin avoidance rules here with spindie
    static void configureDigitaPins() {
        for (auto pbi = gPinBlocksDigital.begin(); pbi != gPinBlocksDigital.end(); pbi++) {
            int numReadPIns =  (pbi->numCtrls < pbi->numReadPins) ? pbi->numCtrls : pbi->numReadPins;
            int numSelPins = pbi->numSelectPins;
            for (auto contactIndx = 0; contactIndx < pbi->numContacts; contactIndx++) {
                int readBase = pbi->pbPinInfo[contactIndx].readBasePin;
                if (pbi->useSelect) {
                    int selBase = pbi->pbPinInfo[contactIndx].selectBasePin;
                    for (auto j = selBase; j < selBase + numSelPins; j++) {
                        checkLegalPin(j, "Bad Sel Pin");
                        // scan pins get configured as output
                        pinMode(j, OUTPUT);
                        fastwrite(j, pbi->activeLow ? HIGH : LOW);
                        AM_DBG(F("Pin"), j, F("Output"));
                    }
                }
                for (auto j = readBase; j < readBase + numReadPIns; j++ ) {
                    checkLegalPin(j, "Bad Read Pin");
                    // read pins get configured as INPUT_PULLUP
                    pinMode(j, pbi->activeLow ? INPUT_PULLUP : INPUT);
                    AM_DBG(F("Pin"), j, F("Pullup"));
                }
            } // contacts loop
        } // pinBlock loop
    }

    // configure analog input pin blocks - set as input with pullup disabled
    // The analog pins merely need to not be in output mode - analogRead and digitalRead will both work on an analog pin in input mode.
    // We disable the pullup here in case somehow we enter with the pullup enabled.
    // analogRead will still work if the pullup is on but the readings will be inaccurate
    static void configureAnalogPins() {
        for (auto pbi = gPinBlocksAnalog.begin(); pbi != gPinBlocksAnalog.end(); pbi++) {
            for (int anPin = pbi->basePin; anPin < pbi->basePin + pbi->numPins; anPin++) {
                checkLegalPin(anPin, "Bad AnalogIn Pin");
                // make sure tha analog pin is actually an analog capable pin
                checkLegalAnalogPin(anPin, "Not AnalogIn");
                pinMode(anPin, INPUT);
                AM_DBG(F("Pin"), anPin, F("AnInput"));
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

    static void scanAnalogPinBlocks() {
        for (auto pbi = gPinBlocksAnalog.begin(); pbi != gPinBlocksAnalog.end(); pbi++) {
            uint8_t ccNum = pbi->baseCCNum;
            for (int anPin = pbi->basePin; anPin < pbi->basePin + pbi->numPins; anPin++, ccNum++) {
                // *** where do we stash the running value?

                // read and convert to float fraction of full range
                // we've got an FPU in the Grand Central and we're gonna use it
                float val = (float)analogRead(anPin) / 65536.0;

                // scaled range is distance between the end guardbands
                // guards are specified as %up from bottom and % down from top
                //  Lower guard specifies the zero point - values below this are clipped to 0
                //  Upper guard is the full range point - values above are clipped to 1.0
                //  Value is translated to fraction of distance between lower and upper guard points

                // normalize guard points to [0, 1.0]
                float lowGuard = pbi->lowEndband / 100.0;
                float hiGuard = pbi->highEndBand / 100.0;

                // scale input value to the interval between the guard points and clamp to [0,1.0]
                float scaledVal = 0;
                if (val > hiGuard) {
                    scaledVal = 1.0;
                }
                else if (val >= lowGuard) {
                    scaledVal = (val - lowGuard) / (hiGuard - lowGuard);  // scale to interval
                }

                // apply simple exponential LPF to scaledVal to get filteredVal
                //float filteredVal = pbi->filterAlpha * scaledVal + (1 - pbi->filterAlpha) * previous_filteredVal;

                // deadband filter needed to prevent excessive messages
                // see if filteredVal has moved away from previously transmitted filteredVal by more than the deadband
                // if so, transmit MIDI CC message and record filteredVal as the new deadband center

                AM_DBG(F("AnPin"), anPin, F("Val"), val);
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

    // simulated 8x8 diode matrix - *** must separately config the scan pins as output
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

    // check for illegal pin based on runtime config
    // spindies on fail - game over because the config will not run successfully
    static void checkLegalPin(int pin, const char * msg) {
        // always-illegal pins: 0-1 Serial / USB port
        if ((pin == 0) || (pin == 1)) _SpinDie(msg, pin);

        // SD card chip select:  pin 4 for SD on eth card (Due), out of band CS on Grand Central so not illegal there
#ifdef ARDUINO_SAM_DUE
        if (pin == 4) _SpinDie(msg, pin);
#endif
        // Ethernet chip select pin 10
        if (gConfig.useEthernet && (pin ==10)) _SpinDie(msg, pin);

        // LED pin 13
        // On the Grand Central it is OK to configure pin 13 as input; it has buffering and can do other functions e.g. SER5
        // On the Due it is technically possible but requires special config of the onboard PIOB and also special code to read;
        // if you try to do it the normal way the LED becomes a weak pulldown on the input.  So we ban it since I think
        // we will ultimately drop Due support.
#ifdef ARDUINO_SAM_DUE
        if (pin == 13) _SpinDie(msg, pin);
#endif

        // LCD I2C pins 20-21
        if (gConfig.useLcd && ((pin == 20) || (pin == 21))) _SpinDie(msg, pin);

        // Ethernet SPI (ICSP headers) pins 50-52
        if (gConfig.useEthernet && pin >= 50 && pin <= 52) _SpinDie(msg, pin);
    }

    // Make sure the specified pin is analog-capable on the target and spindie if not
    static void checkLegalAnalogPin(int pin, const char * msg) {
    #ifdef ARDUINO_SAM_DUE
        if (pin < A0 || pin > A11) _SpinDie(msg, pin);
    #elif defined(ARDUINO_GRAND_CENTRAL_M4)
        // Analog to digital pin numbering is screwy on the M4: there are two disjoint blocks with a dangerous gap.
        // A0-A7 are digital 67-74.  A8-A15 are digital 54-61.  Trying to reference digital 62-66 causes a hard crash.
        if (pin < 54 || pin > 74 || (pin >= 62 && pin <=66)) _SpinDie(msg, pin);
    #else
    #error Unsupported processor type!
    #endif

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
