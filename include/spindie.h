#pragma once
#include <LiquidCrystal_I2C.h>
#include "data.h"

inline void _SpinDie( const char * msg, int val = 0) {
    AM_DBG(F("CRASH"), msg, val);
    if (gConfig.useLcd) {
        char buf[80];
        snprintf(buf, sizeof(buf)-1, "%s %d", msg, val);
        gLcd->pLCD->clear();
        gLcd->lcdMessage("CRASHED");
        gLcd->pLCD->setCursor(0, 1);
        gLcd->lcdMessage(buf);
    }
    while(1);   // the end
}
