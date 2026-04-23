#include <Arduino.h>

#include "glob_gen.h"
#include "data.h"
#include "pin_list.h"


int calcPinBlockSize(int pbIndx) {
    const PinBlock_t *pb = &(gPinBlocks[pbIndx]);
    return (pb->useSelect ? pb->numReadPins * pb->numSelectPins : pb->numReadPins);
}

int calcDebouncerBase(int pbIndx) {
    int dbBase = 0;
    for (int i = 0; i < pbIndx; i++) {
        dbBase += calcPinBlockSize(i);
    }
    return dbBase;
}

void initDebouncerBases() {
    for (int i = 0; i < nPinBlocks; i++) {
        gDebouncerBases[i] = calcDebouncerBase(i);
    }
}

// traverse the pinBlocks and init their debouncers accordingly
void initDebouncers() {
    // need to reset them all, seeing flakiness
    for (int i = 0; i < MAX_DEBOUNCERS; i++) {
        gDebouncers[i].reset();
    }
}

// sped up somewhat by computing the base debouncer index for each pin block at startup for 32-40 bytes of RAM
// cannot precompute for every pin - too much RAM
// currently unused, new scan loop uses the precomputed bases plus an incremental index
int calcDebouncerIndx(int pbIndx, int selectPin, int readPin) {
    int dbIndx = gDebouncerBases[pbIndx];
    const PinBlock_t *pb = &(gPinBlocks[pbIndx]);
    if (pb->useSelect) {
        dbIndx += (selectPin - pb->selectBasePin) * pb->numReadPins + (readPin - pb->readBasePin);
    }
    else {
        dbIndx += readPin - pb->readBasePin;
    }
    return dbIndx;
}


// this is a bit expensive as we have to traverse the debouncer bases list (but it's very short)
int getPinBlockIndxFromDebouncerIndx( int debIndx ) {
    int i = 0;

    while ( (i < nPinBlocks - 1) && (debIndx >= gDebouncerBases[i+1]) ) {
        i++;
    }
    return i;
}

