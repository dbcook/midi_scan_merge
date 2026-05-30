#pragma once

#include "glob_gen.h"
#include "data.h"
#include "ArrayList.h"

#define ANALOG_LPF_FILTERS_INITIAL_ALLOC        // initial allocation size for deque of filters

class AnalogLPF {
    protected:
        int pin = A0;               // pin number of the analog input
        uint8_t initCount = 0;      // don't emit messages until N samples have been seen
        float lowGuard = 0.03;      // fraction of full range (up from the bottom) below which filterVal is clamped to zero
        float highGuard = 0.97;     // fraction of full range (up from the bottom) above which filterVal is clamped to 1.0
        float deadBand = 0.005;     // fractional half-width deadband - filterVal must move outside this range before a message is sent
        float lastSentVal = 0.0;    // filterVal when most recent CC message was sent - defines current center of deadband
        float range = 0.94;         // (highGuard - lowGuard) precomputed for scaling computation (rawVal - lowGuard) / range;
        float alpha = 0.20;         // filterVal = alpha * newval + (1.0 - alpha) * filterVal
        float filterVal = 0;        // running value for exponential LPF
        midi::Channel outChan;      // MIDI chan to which the CC is sent
        midi::DataByte ccNum = 1;   // MIDI CC number to use for the output messages

        static constexpr float fullrange = 65536.0;
        static const uint8_t initSamples = 50;

        static ArrayList<AnalogLPF> analogFilters;

    public:
        AnalogLPF() {}

        AnalogLPF(int pin, midi::DataByte ccNum, midi::Channel outChan, float lowGuard, float highGuard, float deadBand, float alpha ) {
            this->pin = pin;
            this->ccNum = ccNum;
            this->outChan = outChan;
            this->lowGuard = lowGuard;
            this->highGuard = highGuard;
            this->deadBand = deadBand;
            this->alpha = alpha;
            initComputedMembers();
        }

        void initComputedMembers() {
            lastSentVal = 0.0;
            range = highGuard - lowGuard;
            filterVal = 0;
            initCount = 0;
        }

        // causes the filter to restabilize before sending outputs again
        void resetRunningFilter() {
            initCount = 0;
        }

        void setNoteAndChan( midi::DataByte ccNum, midi::Channel outChan) {
            ccNum = ccNum;
            outChan = outChan;
        }

        static void addAnalogLPF(int pin, midi::DataByte ccNum, midi::Channel outChan, float lowGuard, float highGuard, float deadBand, float alpha );
        static void filterAllAnalogInputs();
 

        void processAnalogSample() {
            float val = (float)analogRead(pin) / fullrange;
            // scale to interval, prevent slightly negative values
            val = max((val - lowGuard) / range, 0);
            filterVal = alpha * val + (1.0 - alpha) * filterVal;
            // Stabilization wait for N samples
            // TODO make this a time-based delay
            if (initCount < initSamples) {
                initCount++;
            }
            else {
                if (abs(filterVal - lastSentVal) > deadBand) {
                    // scale float filterVal to MIDI CC value range 0-127
                    midi::DataByte ccVal = min((uint8_t)(filterVal * 128.0), 127);

                    sendCCMessages(ccVal);
                    AM_DBG(F("AN CC"), this->ccNum, F("Val"), ccVal);

                    // recenter deadband
                    lastSentVal = filterVal;
                }
            }
        }

        void sendCCMessages(midi::DataByte val) {
            if (gConfig.useEthernet) {
                gMidiEthOutputInterface->sendControlChange(ccNum, val, outChan);
            }
            // *** TODO Serial and USB transports
        }


};
