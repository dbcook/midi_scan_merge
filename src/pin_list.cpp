#include <Arduino.h>

#include "glob_gen.h"
#include "data.h"

#include "config_features.h"
#include "pin_list.h"

int calcPinBlockSize(int pbIndx) {
    const PinBlockMulti_t *pb = &(gPinBlocksDigital[pbIndx]);
    return pb->numCtrls;
}

int calcDebouncerBase(int pbIndx) {
    int dbBase = 0;
    for (int i = 0; i < pbIndx; i++) {
        dbBase += calcPinBlockSize(i);
    }
    return dbBase;
}

void initDebouncerBases() {
    for (size_t i = 0; i < gPinBlocksDigital.size(); i++) {
        gDebouncerBases.push_back(calcDebouncerBase(i));
    }
}

// Do not call this inside the main scanning loop
int calcNumDigitalInputs() {
    // compute as sum of all pinBlock numCtrls
    int sum = 0;
    for (size_t i = 0; i < gPinBlocksDigital.size(); i++ ) {
        sum += gPinBlocksDigital[i].numCtrls;
    }
    return sum;
}

// Do not call this inside the main scanning loop
int calcNumAnalogInputs() {
    int sum = 0;
    for (size_t i = 0; i < gPinBlocksAnalog.size(); i++ ) {
        sum += gPinBlocksAnalog[i].numPins;
    }
    return sum;
}


// traverse the pinBlocks and init their debouncers accordingly
// *** need to enforce constraint during pinBlock readin that (baseNoteNum + numCtrls - 1) <= MAX_MIDI_NOTE_NUMBER (128)
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
void initMemPinBlocks() {

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

