#include "data.h"
#include "spindie.h"

// The use case here is to display a message about some unrecoverable condition
// and then stop all foreground processing that might prevent the message from
// being seen (i.e. an LCD update), while leaving any background IO active.
//
// Die by spitting out the message on console and/or LCD and then dropping
// into a hard loop calling delay().  That in turn runs yield() which allows
// certain library-implemented background things to continue (hardware and
// library specific).  On AVR type systems the default yield is a
// weak system function that does nothing but can be overridden as desired.
// On ESP8266 systems with WiFi, the WiFi comms will stop if yield() is not called.

void _SpinDie( const char * msg, int val ) {
    AM_DBG(F("CRASH"), msg, val);
    if (gConfig.useLcd) {
        char buf[14];
        snprintf_P(buf, sizeof(buf)-1, PSTR(" %d"), val);
        gLcd->pLCD->clear();
        gLcd->lcdMessage(F("CRASH"));
        gLcd->pLCD->setCursor(0, 1);
        gLcd->lcdMessage(msg);
        gLcd->lcdMessage(buf);
    }
    while(1)
        delay(1000);   // the end
}

// note - snprintf_P format string doesn't interpret %S as documented, it's looking for wchar_t *
void _SpinDie( const __FlashStringHelper * msg, int val) {
    AM_DBG(F("CRASH"), msg, val);
    if (gConfig.useLcd) {
        char buf[14];
        snprintf_P(buf, sizeof(buf)-1, PSTR(" %d"), val);
        gLcd->pLCD->clear();
        gLcd->lcdMessage(F("CRASH"));
        gLcd->pLCD->setCursor(0, 1);
        gLcd->lcdMessage(msg);
        gLcd->lcdMessage(buf);
    }
    while(1)
        delay(1000);   // the end
}

void _SpinDie( const __FlashStringHelper * msg ) {
    AM_DBG(F("CRASH"), msg);
    if (gConfig.useLcd) {
        gLcd->pLCD->clear();
        gLcd->lcdMessage(F("CRASH"));
        gLcd->pLCD->setCursor(0, 1);
        gLcd->lcdMessage(msg);
    }    
    while(1)
        delay(1000);   // the end
}

void _SpinDie( const char * msg ) {
    AM_DBG(F("CRASH"), msg);
    if (gConfig.useLcd) {
        gLcd->pLCD->clear();
        gLcd->lcdMessage(F("CRASH"));
        gLcd->pLCD->setCursor(0, 1);
        gLcd->lcdMessage(msg);
    }    
    while(1)
        delay(1000);   // the end
}
