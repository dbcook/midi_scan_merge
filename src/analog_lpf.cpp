
#include "glob_gen.h"
#include "analog_lpf.h"


// instantiate static members
ArrayList<AnalogLPF> AnalogLPF::analogFilters;

// Operate on the embedded static list of AnalogLPF objects

void AnalogLPF::addAnalogLPF(int pin, midi::DataByte ccNum, midi::Channel outChan, float lowGuard, float highGuard, float deadBand, float alpha ) {
    AnalogLPF nf(pin, ccNum, outChan, lowGuard, highGuard, deadBand, alpha);
    analogFilters.add(nf);
}

void AnalogLPF::filterAllAnalogInputs() {
    for (auto filt = analogFilters.begin(); filt != analogFilters.end(); filt++) {
        filt->processAnalogSample();
    }
}
