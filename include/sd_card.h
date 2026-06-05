// Class for managing the AdaFruit Grand Central M4 onboard SD card

#pragma once

#include <Arduino.h>
#include <SdFat.h>

#include "glob_gen.h"
#include "spindie.h"

#define SPI_RATE SPI_FULL_SPEED     // alt SPI_HALF_SPEED

#if defined( ARDUINO_SAM_DUE )
    const int chipSelect = 4;
#elif defined( ARDUINO_GRAND_CENTRAL_M4 )
    const int chipSelect = 53;
#endif

class SdCardMgr {

protected:
    static SdFat sd;

public:
    SdCardMgr() {}

    static void initForReadOnly() {
        // this is going to fail if the card is not physically present
        if (!sd.begin( chipSelect, SPI_RATE )) {
            uint8_t errcode = sd.card()->errorCode();
            if (errcode) {
                int32_t errdata = sd.card()->errorData();
                if (errdata) {
                    char buf[40];
                    snprintf_P(buf, sizeof(buf)-1, PSTR("SD InitFail %d\nErr Data %ld"), errcode, errdata);
                    _SpinDie(buf);

                }
            }

        }

    }

};

