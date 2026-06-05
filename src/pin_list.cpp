#include <Arduino.h>

#include "glob_gen.h"
#include "data.h"

#include "config_features.h"
#include "pin_list.h"
#include "spindie.h"

// --- Static class member functions ---

// check for patently illegal pin based on runtime config
// spindies on fail - game over because the config will not run successfully
void PinList::checkLegalPin(int pin, const char * msg) {
    // always-illegal pins: 0-1 Serial / USB port
    // it's theoretically possible to use these pins if the bootloader doesn't use them, and we emit no
    // console msgs or send them out a dedicated USB serial port.
    if ((pin == 0) || (pin == 1)) _SpinDie(msg);

    // SD card chip select on Ethernet Shield 2:  pin 4
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

    // SD card chip select for Grand Central
#if defined( ADAFRUIT_GRAND_CENTRAL_M4 )
    if (pin == 53) _SpinDie(msg, pin);
#endif
}

void PinList::checkLegalAnalogPin(int pin, const char * msg) {
#ifdef ARDUINO_SAM_DUE
    if (pin < A0 || pin > A11) _SpinDie(msg, pin);
#elif defined(ARDUINO_GRAND_CENTRAL_M4)
    // Analog to digital pin numbering is screwy on the GCM4: there are two disjoint blocks with a dangerous gap.
    // A0-A7 are digital 67-74.  A8-A15 are digital 54-61.  Trying to reference digital 62-66 causes a hard crash.
    if (pin < 54 || pin > 74 || (pin >= 62 && pin <=66)) _SpinDie(msg, pin);
#else
#error Unsupported processor type!
#endif

}

int PinList::calcDigitalPinBlockSize(int pbIndx) {
    const PinBlockMulti_t *pb = &(gPinBlocksDigital[pbIndx]);
    return pb->numCtrls;
}

// Do not call this inside the main scanning loop
int PinList::calcNumDigitalInputs() {
    // compute as sum of all pinBlock numCtrls
    int sum = 0;
    for (size_t i = 0; i < gPinBlocksDigital.size(); i++ ) {
        sum += gPinBlocksDigital[i].numCtrls;
    }
    return sum;
}

// Do not call this inside the main scanning loop
int PinList::calcNumAnalogInputs() {
    int sum = 0;
    for (size_t i = 0; i < gPinBlocksAnalog.size(); i++ ) {
        sum += gPinBlocksAnalog[i].numPins;
    }
    return sum;
}


// --- helpers ---
// these hover between belonging in PinList or in Debouncer, but lean toward Debouncer using PinBlock facilities


int calcDebouncerBase(int pbIndx) {
    int dbBase = 0;
    for (int i = 0; i < pbIndx; i++) {
        dbBase += PinList::calcDigitalPinBlockSize(i);
    }
    return dbBase;
}

void initDebouncerBases() {
    for (size_t i = 0; i < gPinBlocksDigital.size(); i++) {
        gDebouncerBases.push_back(calcDebouncerBase(i));
    }
}



// traverse the pinBlocks and init their debouncers accordingly
// and make sure we don't go off the end of the debouncers allocation
void initDebouncers() {
    // need to reset them all
    for (int i = 0; i < MAX_DEBOUNCERS; i++) {
        gDebouncers[i].reset();
    }
    // Make sure debouncer block bases are set
    initDebouncerBases();
    // traverse pinBlocks and poke noteNum and midiOutChan into the debouncers
    for (size_t i = 0; i < gPinBlocksDigital.size(); i++) {
        int dbase = gDebouncerBases[i];
        const PinBlockMulti_t *pb = &(gPinBlocksDigital[i]);
        if ( (dbase + pb->numCtrls) > MAX_DEBOUNCERS) _SpinDie(F("Too many debouncers"), dbase + pb->numCtrls);
        for (int j = 0; j < pb->numCtrls; j++) {
            gDebouncers[dbase + j].setNoteAndChan(pb->baseMidiNoteNum + j, pb->midiOutChan);
            gDebouncers[dbase + j].setDebounceTimes(pb->attackDebounceMsec, pb->releaseDebounceMsec);
        }
    }
}


void initAnalogInputFilters() {

}

// Set up in-memory list for each pinblock group with switchable source
//      hardcoded defs in flash
//      config read from SD card
// The config source has to be a compile time switch else the eggs and chickens get in a shouting match
void PinList::initMemPinBlocks() {

#if PIN_CONFIG_SOURCE == flash
    for (size_t i = 0; i < nFlashPinBlocksMulti; i++) {
        const PinBlockMulti_t * pbsrc = &gFlashPinBlocksMulti[i];
        gPinBlocksDigital.add(*pbsrc);
    }
    for (size_t i = 0; i < nFlashPinBlocksAnalogRead; i++) {
        const PinBlockAnalogRead_t * pbsrc = &gFlashPinBlocksAnalogRead[i];
        gPinBlocksAnalog.add(*pbsrc);
    }
#elif PIN_CONFIG_SOURCE == config
    // *** parse YAML from the SD card
#else
#error Unsupported pinblock source!
#endif

}

