#pragma once

#include<Arduino.h>
#include<variant.h>
#include "glob_gen.h"

// RESTORE MISSING PLATFORMIO HARDWARE UART BOARD SUPPORT FEATURES in AdaFruit Grand Central M4 Express variant.h / variant.cpp
//
// The current Arduino board support for the AdaFruit Grand Central M4 Express only implements one
// out of the four hardware serial ports on the module.  This mini-library fixes that.
//
// This material really should be added to variant.h and variant.cpp in the BSP unless there is some reason
// not to do that that I don't know about.
//
// Native USB serial (Serial_ Serial) has no breakout pins and uses a different class object.
// However you can write to it to get debug monitor or other output sent back out the native USB port.
//
// Board applicability:
//   I have chosen to use the board level selector symbol because other boards with fewer GPIO pins - even
//   if based on the same MCU - may well have fewer usable hardware UARTs and different Sercom mapping.
//
//   Much information can be gleaned from github.com/platformio/platform-atmelsam/boards
//   You need to look at <boardname>.json to find the symbols, and you need to find the board's variant.h and variant.cpp
//   to see the Sercom mappings and pin definitions that will let you figure out how many hardware UARTs actually exist.
//
//   The Grand Central M4 has board symbols ARDUINO_GRAND_CENTRAL_M4 and ADAFRUIT_GRAND_CENTRAL_M4, MCU symbol __SAMD51P20A__ and architecture __SAMD51__
// 
//   The Grand Central has 8 sercoms - 4 are hardware UARTs, SPI is sercom7 and SPI1 is sercom2, WIRE is sercom3 and WIRE1 is sercom6.
//   Note that the TX and RX pin order for Serial1 is backwards - RX pin comes first - from the order used for the other 3 ports.
//
//      Serial1 is pins 0 RX and 1 TX   (sercom0) - defined for us in variant.h / variant.cpp
//      Serial2 is pins 18 TX and 19 RX (sercom4)
//      Serial3 is pins 16 TX and 17 RX (sercom1)
//      Serial4 is pins 14 TX and 15 RX (sercom5)

#if defined( ARDUINO_GRAND_CENTRAL_M4 )

extern Uart Serial2;
extern Uart Serial3;
extern Uart Serial4;

#endif