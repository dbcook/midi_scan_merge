#pragma once
#include <Arduino.h>
#include <AppleMIDI.h>
#include "glob_gen.h"

// Debug console system
// MUST be included after AppleMidi.h since it nukes AM_DBG symbols unless we let it totally take over the console port.
// All console code vanishes when USE_DEBUG_PRINT is false
// If USE_DEBUG_PRINT is true, the MIDI shield (if present) must not use Serial but must use Serial1 - Serial3
// ALWAYS use F("msg") to send the literals to flash instead of RAM

#define USE_DEBUG_PRINT true

#if USE_DEBUG_PRINT
const unsigned long consoleBaudRate = 115200;
EXTERN HardwareSerial *Console
#ifdef GEN_GLOBALS
= &Serial
#endif
;

#define Console_print(...) Console->print(__VA_ARGS__)
#define Console_println(...) Console->println(__VA_ARGS__); Console->flush()
#define Console_flush(...) Console->flush()

// modification of the AppleMidi AM_DBG templates from AppleMIDI_Debug.h
// Make sure AppleMIDI_Debug.h is NOT included by not defining SerialMon
// and kill macros that it defines if undefined(SerialMon)
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

