#pragma once
#include <Arduino.h>
#include "glob_gen.h"

// Debug console system
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
#define Console_println(...) Console->println(__VA_ARGS__)
#define Console_flush Console->flush()
#else
#define Console_print(...)
#define Console_println(...)
#define Console_flush(...)
#endif

