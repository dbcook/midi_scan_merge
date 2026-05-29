#pragma once
#include <Arduino.h>
#include <AppleMIDI.h>
#include "glob_gen.h"
#include "serial_gcm4.h"

// Debug console system
// MUST be included AFTER AppleMidi.h since that file nukes AM_DBG symbols unless we let it totally take over the console port.
//
// All console code vanishes when USE_DEBUG_PRINT is false
// If USE_DEBUG_PRINT is true, the MIDI shield (if present) must avoid the hardware UART used for the console.
// ALWAYS use F("msg") to store the literals to flash instead of RAM

#define USE_DEBUG_PRINT true

#if USE_DEBUG_PRINT

const unsigned long consoleBaudRate = 115200;   // must match monitor_speed in platformio.h

// Platform Specifics
//
// Assigning the console output port is platform dependent, and rather messy due to port naming irregularity.
//
// AdaFruit Grand Central M4:
// To send console messages back out the USB port on Grand Central we need to use the USB port object
//    Serial_ Serial
// while the four hardware UARTs are
//    Uart Serial1, Serial2, Serial3, Serial4
// To use Serial2 through Serial4 you have to use the serial_gcm4.h / .cpp I wrote to extend what variant.h / .cpp provide
//
// Arduino Due:
//    On Due the hardware UARTs are all
//      HardwareSerial Serial, Serial1, Serial2, Serial3
//    If you put the console on "Serial", the messages go back out the programming port USB but I believe this is tied to GPIO pins 0-1.
//
// PlatformIO board identification symbols
//     Grand Central: ARDUINO_GRAND_CENTRAL_M4 ( see https://github.com/platformio/platform-atmelsam/blob/master/boards/adafruit_grandcentral_m4.json )
//     Arduino Due  : ARDUINO_SAM_DUE

#if defined( ARDUINO_SAM_DUE )

EXTERN HardwareSerial *Console
#ifdef GEN_GLOBALS
= &Serial
#endif
;

#elif defined( ARDUINO_GRAND_CENTRAL_M4 )

// console on native USB port "Serial_ Serial"
EXTERN Serial_ *Console
// alternative console on "Uart Serial1" (pins 0,1), needed when using native USB MIDI transport output
// To use this you have to hook up a separate USB-serial adapter to the GPIO pins for the Sercom.
// The TX/RX pins are the same as other Arduino large format boards but they are named Serial1 - Serial4
// instead of Serial, Serial1, Serial2, Serial3.
// EXTERN Uart *Console = &Serial1
#ifdef GEN_GLOBALS
= &Serial
#endif
;

#else
#error Unsupported board type!
#endif


#define Console_print(...) Console->print(__VA_ARGS__)
#define Console_println(...) Console->println(__VA_ARGS__); Console->flush()
#define Console_flush(...) Console->flush()

// modification of the AppleMidi AM_DBG templates from AppleMIDI_Debug.h
// Make sure AppleMIDI_Debug.h is NOT included by not defining SerialMon
// and kill macros that it defines when undefined(SerialMon)
#undef SerialMon

#undef AM_DBG_SETUP       // no implementation, now controlled by USE_DEBUG_PRINT
#undef AM_DBG
#undef AM_DBG_PLAIN

namespace {

template <typename T>
static void AM_DBG_PLAIN(T last) {
  Console_println(last);
}

template <typename T, typename... Args>
static void AM_DBG_PLAIN(T head, Args... tail) {
  Console->print(head);
  Console->print(' ');
  AM_DBG_PLAIN(tail...);
}

template <typename... Args>
static void AM_DBG(Args... args) {
    AM_DBG_PLAIN(args...);
}

}  // namespace


#else
#define AM_DBG_SETUP(...)
#define AM_DBG_PLAIN(...)
#define AM_DBG(...)
#define Console_print(...)
#define Console_println(...)
#define Console_flush(...)
#endif

