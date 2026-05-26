#pragma once

#include "deque.h"
#include <LiquidCrystal_I2C.h>


// Constructor does some heap allocations, so best to do at init time
class LcdDisplay {
    public:
        LcdDisplay( uint8_t i2c_addr = 0x27, uint8_t ncols = 20, uint8_t nrows = 4, uint8_t msgBufSize = 10) {
            pLCD = new LiquidCrystal_I2C(i2c_addr, ncols, nrows);
            pMsgBuf = new Deque<LcdMsg_t>(msgBufSize);
            for (auto i = 0; i < msgBufSize; i++) {
                (*pMsgBuf)[i].pTxt = new char[ncols+1];
            }
        }
        LiquidCrystal_I2C *pLCD;        // exposed so you can call native lib methods

        void lcdMessage( const char * txt, uint8_t col, uint8_t row, bool foreground = true ) {
            if (foreground) {
                // bypass queue, msg sent directly in foreground with blocking IO
                // TODO - limit length to remainder of specified row?
                pLCD->setCursor(col, row);
                pLCD->print(txt);
            }
            else {
                // queue msg for background send

            }
        }

        // print message at current cursor
        void lcdMessage( const char * txt, bool foreground = true) {
            if (foreground) {
                pLCD->print(txt);
            }
            else {
                // queue it up - need to have queue entry know if it is positioned or at-cursor
            }

        }

        // a few wrappers for common manipulations from underlying lib
        void init(bool backlight = true) {
            pLCD->init();
            // let's crank up the clock (experimental, might need 4.7k pullups on SCL and SDA)
            // works fine at 200 KHz (2x normal) with no pullups
            Wire.setClock(200000);
            if (backlight) {
                pLCD->backlight();
            }
        }

    protected:
        typedef struct {
            char * pTxt;
            uint8_t row;
            uint8_t col;
            bool foreground;
        } LcdMsg_t;

        Deque<LcdMsg_t> *pMsgBuf;

};