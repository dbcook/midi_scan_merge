#pragma once
#include <LiquidCrystal_I2C.h>
#include "data.h"

inline void _SpinDie( const char * msg, int val = 0) {
    AM_DBG(F("CRASH"), msg, val);
    if (gConfig.useLcd) {
        char buf[14];
        snprintf_P(buf, sizeof(buf)-1, PSTR(" %d"), val);
        gLcd->pLCD->clear();
        gLcd->lcdMessage(F("CRASHED"));
        gLcd->pLCD->setCursor(0, 1);
        gLcd->lcdMessage(msg);
        gLcd->lcdMessage(buf);
    }
    while(1);   // the end
}

// note - snprintf_P format string doesn't interpret %S as documented, it's looking for wchar_t *
inline void _SpinDie( const __FlashStringHelper * msg, int val = 0) {
    AM_DBG(F("CRASH"), msg, val);
    if (gConfig.useLcd) {
        char buf[1];
        snprintf_P(buf, sizeof(buf)-1, PSTR(" %d"), val);
        gLcd->pLCD->clear();
        gLcd->lcdMessage(F("CRASHED"));
        gLcd->pLCD->setCursor(0, 1);
        gLcd->lcdMessage(msg);
        gLcd->lcdMessage(buf);
    }
    while(1);   // the end
}

inline void _SpinDie( const __FlashStringHelper * msg ) {
    AM_DBG(F("CRASH"), msg);
    if (gConfig.useLcd) {
        gLcd->pLCD->clear();
        gLcd->lcdMessage(F("CRASHED"));
        gLcd->pLCD->setCursor(0, 1);
        gLcd->lcdMessage(msg);
    }    
}

inline void _SpinDie( const char * msg ) {
    AM_DBG(F("CRASH"), msg);
    if (gConfig.useLcd) {
        gLcd->pLCD->clear();
        gLcd->lcdMessage(F("CRASHED"));
        gLcd->pLCD->setCursor(0, 1);
        gLcd->lcdMessage(msg);
    }    
}
