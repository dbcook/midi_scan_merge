#pragma once

#include <Arduino.h>

#include "glob_gen.h"

#define THROW_ERROR_IF_NOT_FAST
#include "digitalWriteFast.h"

// A series of functions using the fast IO read/write library ArminJo/digitalWriteFast to read
// and write Arduino IO pins much faster than the standard digitalRead() and digitalWrite().
//
// Here we make use of the IO library ArminJo/digitalWriteFast.  That library provides much faster
// pin read/write operations by eliminating the sanity checks in digitalWrite and digitalRead,
// but only works for pin numbers that resolve to compile-time constants.
// That is not sufficient for our use case, where we must be able to handle
// variable pin numbers to accommodate fully flexible I/O pin mappings.
//
// NOTE: There are multiple forks of digitalWriteFast on GitHub; the ArminJo version is the
// most up to date and is available in the PlatformIO library manager.
//
// On Arduino, to prevent the function ptr arrays from being stored in static RAM you
// have to use the PROGMEM directive on them in addition to declaring them 'const'.
// Thanks to this discovery, this library now consumes zero static RAM.
//
// Arduino Mega 2560 Details
// -------------------------
// The Arduino Mega 2560 has physical IO registers that can be read to get 8 bits at once.
// Typically in a matrix I/O situation we will read either 4 or 8 pins per scan pin,
// so there would be a further advantage to align the read pins with register boundaries; however
// this imposes very great constraints on IO pin mapping, and it appears that it would
// not be a lot faster in an 8x8 diode matrix scan situation.
//
// For reference, the Mega 2560 register to IO pin mappings are as follows.
// Note that there are discontiguous regions and some registers with reverse bit order.
// In addition, PORTE and PORTJ contain signals that do not appear as IO pins on the Arduino Mega 2560 board.
//
// see https://docs.arduino.cc/retired/hacking/hardware/PinMapping2560/ for documentation
// of the Arduino pins to ATMega 2560 chip pins & ports.
// Here is a summary of the Mega 2560 pin-to-port mapping.
//
//  PORTE       0-3     5
//  PORTB       10-13   50-53
//  PORTJ       14-15
//  PORTH       16-17   6-9     7-9 are reverse bit order on bits 4-6, pin 6 is bit 3 so it goes 6987
//  PORTD       18-21   38      pretty much random bit placement 18 = bit 3, pin 19 = bit 2, 20 = bit 1, 21 = bit 0, pin 38 = bit 7
//  PORTA       22-29
//  PORTC       30-37           reverse bit order
//  PORTG       39-41   4       reverse bit order
//  PORTL       42-49           reverse bit order
//  PORTK       62-69           (analog A8-A15)
//  PORTF       54-61           (analog A0-A7)

// function pointer types for digitalFastRead/Write wrappers
typedef int (*fastRdFuncPtr)();
typedef void (*fastWrFuncPtr)(int);

// Wrapper functions for digitalReadFast whose addresses can be taken and
// put into an array.
// The key point is that each of these functions invokes digitalReadFast/WriteFast with an
// arg that is a compile-time constant.
int fastread_2() {return digitalReadFast(2);}
int fastread_3() {return digitalReadFast(3);}
int fastread_4() {return digitalReadFast(4);}
int fastread_5() {return digitalReadFast(5);}
int fastread_6() {return digitalReadFast(6);}
int fastread_7() {return digitalReadFast(7);}
int fastread_8() {return digitalReadFast(8);}
int fastread_9() {return digitalReadFast(9);}
int fastread_10() {return digitalReadFast(10);}
int fastread_11() {return digitalReadFast(11);}
int fastread_12() {return digitalReadFast(12);}
int fastread_13() {return digitalReadFast(13);}
int fastread_14() {return digitalReadFast(14);}
int fastread_15() {return digitalReadFast(15);}
int fastread_16() {return digitalReadFast(16);}
int fastread_17() {return digitalReadFast(17);}
int fastread_18() {return digitalReadFast(18);}
int fastread_19() {return digitalReadFast(19);}
int fastread_20() {return digitalReadFast(20);}
int fastread_21() {return digitalReadFast(21);}
int fastread_22() {return digitalReadFast(22);}
int fastread_23() {return digitalReadFast(23);}
int fastread_24() {return digitalReadFast(24);}
int fastread_25() {return digitalReadFast(25);}
int fastread_26() {return digitalReadFast(26);}
int fastread_27() {return digitalReadFast(27);}
int fastread_28() {return digitalReadFast(28);}
int fastread_29() {return digitalReadFast(29);}
int fastread_30() {return digitalReadFast(30);}
int fastread_31() {return digitalReadFast(31);}
int fastread_32() {return digitalReadFast(32);}
int fastread_33() {return digitalReadFast(33);}
int fastread_34() {return digitalReadFast(34);}
int fastread_35() {return digitalReadFast(35);}
int fastread_36() {return digitalReadFast(36);}
int fastread_37() {return digitalReadFast(37);}
int fastread_38() {return digitalReadFast(38);}
int fastread_39() {return digitalReadFast(39);}
int fastread_40() {return digitalReadFast(40);}
int fastread_41() {return digitalReadFast(41);}
int fastread_42() {return digitalReadFast(42);}
int fastread_43() {return digitalReadFast(43);}
int fastread_44() {return digitalReadFast(44);}
int fastread_45() {return digitalReadFast(45);}
int fastread_46() {return digitalReadFast(46);}
int fastread_47() {return digitalReadFast(47);}
int fastread_48() {return digitalReadFast(48);}
int fastread_49() {return digitalReadFast(49);}
int fastread_50() {return digitalReadFast(50);}
int fastread_51() {return digitalReadFast(51);}
int fastread_52() {return digitalReadFast(52);}
int fastread_53() {return digitalReadFast(53);}
int fastread_54() {return digitalReadFast(54);}
int fastread_55() {return digitalReadFast(55);}
int fastread_56() {return digitalReadFast(56);}
int fastread_57() {return digitalReadFast(57);}
int fastread_58() {return digitalReadFast(58);}
int fastread_59() {return digitalReadFast(59);}
int fastread_60() {return digitalReadFast(60);}
int fastread_61() {return digitalReadFast(61);}
int fastread_62() {return digitalReadFast(62);}
int fastread_63() {return digitalReadFast(63);}
int fastread_64() {return digitalReadFast(64);}
int fastread_65() {return digitalReadFast(65);}
int fastread_66() {return digitalReadFast(66);}
int fastread_67() {return digitalReadFast(67);}
int fastread_68() {return digitalReadFast(68);}
int fastread_69() {return digitalReadFast(69);}

// const array of pointers to the wrapper functions
// have to use PROGMEM to force this into flash
EXTERN const fastRdFuncPtr frPtrs[]
#ifdef GEN_GLOBALS
PROGMEM = {
    NULL,
    NULL,
    fastread_2,
    fastread_3,
    fastread_4,
    fastread_5,
    fastread_6,
    fastread_7,
    fastread_8,
    fastread_9,
    fastread_10,
    fastread_11,
    fastread_12,
    fastread_13,
    fastread_14,
    fastread_15,
    fastread_16,
    fastread_17,
    fastread_18,
    fastread_19,
    fastread_20,
    fastread_21,
    fastread_22,
    fastread_23,
    fastread_24,
    fastread_25,
    fastread_26,
    fastread_27,
    fastread_28,
    fastread_29,
    fastread_30,
    fastread_31,
    fastread_32,
    fastread_33,
    fastread_34,
    fastread_35,
    fastread_36,
    fastread_37,
    fastread_38,
    fastread_39,
    fastread_40,
    fastread_41,
    fastread_42,
    fastread_43,
    fastread_44,
    fastread_45,
    fastread_46,
    fastread_47,
    fastread_48,
    fastread_49,
    fastread_50,
    fastread_51,
    fastread_52,
    fastread_53,
    fastread_54,
    fastread_55,
    fastread_56,
    fastread_57,
    fastread_58,
    fastread_59,
    fastread_60,
    fastread_61,
    fastread_62,
    fastread_63,
    fastread_64,
    fastread_65,
    fastread_66,
    fastread_67,
    fastread_68,
    fastread_69
 }
 #endif
;

// Read an arbitrary pin as quickly as possible
// p can be a variable here
int fastread(int p) {
    return frPtrs[p]();
}

// wrapper functions for digitalWriteFast whose addresses can be taken and
// put into an array
void fastwrite_2(int v) {digitalWriteFast(2, v);}
void fastwrite_3(int v) {digitalWriteFast(3, v);}
void fastwrite_4(int v) {digitalWriteFast(4, v);}
void fastwrite_5(int v) {digitalWriteFast(5, v);}
void fastwrite_6(int v) {digitalWriteFast(6, v);}
void fastwrite_7(int v) {digitalWriteFast(7, v);}
void fastwrite_8(int v) {digitalWriteFast(8, v);}
void fastwrite_9(int v) {digitalWriteFast(9, v);}
void fastwrite_10(int v) {digitalWriteFast(10, v);}
void fastwrite_11(int v) {digitalWriteFast(11, v);}
void fastwrite_12(int v) {digitalWriteFast(12, v);}
void fastwrite_13(int v) {digitalWriteFast(13, v);}
void fastwrite_14(int v) {digitalWriteFast(14, v);}
void fastwrite_15(int v) {digitalWriteFast(15, v);}
void fastwrite_16(int v) {digitalWriteFast(16, v);}
void fastwrite_17(int v) {digitalWriteFast(17, v);}
void fastwrite_18(int v) {digitalWriteFast(18, v);}
void fastwrite_19(int v) {digitalWriteFast(19, v);}
void fastwrite_20(int v) {digitalWriteFast(20, v);}
void fastwrite_21(int v) {digitalWriteFast(21, v);}
void fastwrite_22(int v) {digitalWriteFast(22, v);}
void fastwrite_23(int v) {digitalWriteFast(23, v);}
void fastwrite_24(int v) {digitalWriteFast(24, v);}
void fastwrite_25(int v) {digitalWriteFast(25, v);}
void fastwrite_26(int v) {digitalWriteFast(26, v);}
void fastwrite_27(int v) {digitalWriteFast(27, v);}
void fastwrite_28(int v) {digitalWriteFast(28, v);}
void fastwrite_29(int v) {digitalWriteFast(29, v);}
void fastwrite_30(int v) {digitalWriteFast(30, v);}
void fastwrite_31(int v) {digitalWriteFast(31, v);}
void fastwrite_32(int v) {digitalWriteFast(32, v);}
void fastwrite_33(int v) {digitalWriteFast(33, v);}
void fastwrite_34(int v) {digitalWriteFast(34, v);}
void fastwrite_35(int v) {digitalWriteFast(35, v);}
void fastwrite_36(int v) {digitalWriteFast(36, v);}
void fastwrite_37(int v) {digitalWriteFast(37, v);}
void fastwrite_38(int v) {digitalWriteFast(38, v);}
void fastwrite_39(int v) {digitalWriteFast(39, v);}
void fastwrite_40(int v) {digitalWriteFast(40, v);}
void fastwrite_41(int v) {digitalWriteFast(41, v);}
void fastwrite_42(int v) {digitalWriteFast(42, v);}
void fastwrite_43(int v) {digitalWriteFast(43, v);}
void fastwrite_44(int v) {digitalWriteFast(44, v);}
void fastwrite_45(int v) {digitalWriteFast(45, v);}
void fastwrite_46(int v) {digitalWriteFast(46, v);}
void fastwrite_47(int v) {digitalWriteFast(47, v);}
void fastwrite_48(int v) {digitalWriteFast(48, v);}
void fastwrite_49(int v) {digitalWriteFast(49, v);}
void fastwrite_50(int v) {digitalWriteFast(50, v);}
void fastwrite_51(int v) {digitalWriteFast(51, v);}
void fastwrite_52(int v) {digitalWriteFast(52, v);}
void fastwrite_53(int v) {digitalWriteFast(53, v);}
void fastwrite_54(int v) {digitalWriteFast(54, v);}
void fastwrite_55(int v) {digitalWriteFast(55, v);}
void fastwrite_56(int v) {digitalWriteFast(56, v);}
void fastwrite_57(int v) {digitalWriteFast(57, v);}
void fastwrite_58(int v) {digitalWriteFast(58, v);}
void fastwrite_59(int v) {digitalWriteFast(59, v);}
void fastwrite_60(int v) {digitalWriteFast(60, v);}
void fastwrite_61(int v) {digitalWriteFast(61, v);}
void fastwrite_62(int v) {digitalWriteFast(62, v);}
void fastwrite_63(int v) {digitalWriteFast(63, v);}
void fastwrite_64(int v) {digitalWriteFast(64, v);}
void fastwrite_65(int v) {digitalWriteFast(65, v);}
void fastwrite_66(int v) {digitalWriteFast(66, v);}
void fastwrite_67(int v) {digitalWriteFast(67, v);}
void fastwrite_68(int v) {digitalWriteFast(68, v);}
void fastwrite_69(int v) {digitalWriteFast(69, v);}

// const array of pointers to the wrapper functions
// have to use PROGMEM to force this into flash

EXTERN const fastWrFuncPtr fwPtrs[]
#ifdef GEN_GLOBALS
PROGMEM = {
    NULL,
    NULL,
    fastwrite_2,
    fastwrite_3,
    fastwrite_4,
    fastwrite_5,
    fastwrite_6,
    fastwrite_7,
    fastwrite_8,
    fastwrite_9,
    fastwrite_10,
    fastwrite_11,
    fastwrite_12,
    fastwrite_13,
    fastwrite_14,
    fastwrite_15,
    fastwrite_16,
    fastwrite_17,
    fastwrite_18,
    fastwrite_19,
    fastwrite_20,
    fastwrite_21,
    fastwrite_22,
    fastwrite_23,
    fastwrite_24,
    fastwrite_25,
    fastwrite_26,
    fastwrite_27,
    fastwrite_28,
    fastwrite_29,
    fastwrite_30,
    fastwrite_31,
    fastwrite_32,
    fastwrite_33,
    fastwrite_34,
    fastwrite_35,
    fastwrite_36,
    fastwrite_37,
    fastwrite_38,
    fastwrite_39,
    fastwrite_40,
    fastwrite_41,
    fastwrite_42,
    fastwrite_43,
    fastwrite_44,
    fastwrite_45,
    fastwrite_46,
    fastwrite_47,
    fastwrite_48,
    fastwrite_49,
    fastwrite_50,
    fastwrite_51,
    fastwrite_52,
    fastwrite_53,
    fastwrite_54,
    fastwrite_55,
    fastwrite_56,
    fastwrite_57,
    fastwrite_58,
    fastwrite_59,
    fastwrite_60,
    fastwrite_61,
    fastwrite_62,
    fastwrite_63,
    fastwrite_64,
    fastwrite_65,
    fastwrite_66,
    fastwrite_67,
    fastwrite_68,
    fastwrite_69
}
#endif
;

// write an arbitrary pin with any value
// p (pin) and v (value) can both be variables
void fastwrite(int p, int v) {
    fwPtrs[p](v);
}

//---------------------------------------------------------------
// functions that might be useful later for specific applications
//---------------------------------------------------------------

#if 0
//---------------------------------------------------------------
// read entire ports, extract pins later
//---------------------------------------------------------------
// Functions to read entire Arm2560 ports
// These would be useful for reading diode matrix read lines on certain specified boundaries
// where the whole port is mapped to a contiguous set of pins.
// Probably not that useful in the general case vs just selecting which digitalFastRead to call

// reads portA: pins 22-29
// order (bits 7 to 0):                 29  28  27  26  25  24  23  22
inline uint8_t fastread_portA() {
    return PORTA;
}

// reads portB: pins 10-13 and 50-53
// order (bits 7 to 0):                 13  22  11  10  50  51  52  53
inline uint8_t fastread_portB() {
    return PORTB;
}

inline uint8_t fastread_portC() {
    return PORTC;
}

inline uint8_t fastread_portD() {
    return PORTD;
}

inline uint8_t fastread_portE() {
    return PORTE;
}

inline uint8_t fastread_portF() {
    return PORTF;
}

inline uint8_t fastread_portG() {
    return PORTG;
}

inline uint8_t fastread_portH() {
    return PORTH;
}

inline uint8_t fastread_portJ() {
    return PORTJ;
}

inline uint8_t fastread_portK() {
    return PORTK;
}

inline uint8_t fastread_portL() {
    return PORTL;
}
#endif

#if 0
// (these methods not enabled b/c they require 0.5K of RAM)

//-----------------------------------------------------------------------
// read ports and extract several pins into a static buffer per port read
//-----------------------------------------------------------------------
// this is well suited to reading diode matrix read pins on specified boundaries
// but we would need too many functions to handle all possible boundaries

// The static read buffer consumes some RAM on a small arduino
#define MAX_IO_PINS_MEGA_2560 70
EXTERN uint8_t gPinReadBuf[MAX_IO_PINS_MEGA_2560];

inline void readPins_22_29() {
    // likely the fastest way - extracts the values for 8 pins with one port read
    // by reading the port once and masking bits using bit position macros from digitalFastWrite
    uint8_t val = *__digitalPinToPINReg(22);
    int n = 22;
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(22));
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(23));
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(24));
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(25));
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(26));
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(27));
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(28));
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(29));
}

inline void readPins_30_37() {
    uint8_t val = *__digitalPinToPINReg(30);
    int n = 30;
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(30));
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(31));
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(32));
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(33));
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(34));
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(35));
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(36));
    gPinReadBuf[n++] = val & (1 << __digitalPinToBit(37));
}

//----------------------------------------------------------------------------
// read all pins into buffer using digitalFastRead for each pin
//----------------------------------------------------------------------------
// This is not as fast as the previous method which will keep the PORTx value
// in a CPU register while masking the various bits, but maybe close enough
// This reads the register every time, taking an extra 14 clocks per 8 pins vs
// reading the register once and extracting the 8 bits via AND masks.

// If this takes 4 cycles per read/store operation, it will take ~12.5 usec to read everything
// reads 49 digital pins, skipping 0, 1, 4, 10 and 13.
// This is all the pins you can possibly use with an Ethernet Shield taking up 4 and 10.
// Sharding:
//   Using all pins, ranges for 8x8 diode matrix blocks are 2-20, 21-36, 37-52
//   A 4th 8x8 could live on the 16 analog pins
void readAllMega2560DigitalInputPins() {
    int pin;
    gPinReadBuf[2] = digitalReadFast(2);
    gPinReadBuf[3] = digitalReadFast(3);
    pin = 5;
    gPinReadBuf[pin++] = digitalReadFast(5);
    gPinReadBuf[pin++] = digitalReadFast(6);
    gPinReadBuf[pin++] = digitalReadFast(7);
    gPinReadBuf[pin++] = digitalReadFast(8);
    gPinReadBuf[pin]   = digitalReadFast(9);
    gPinReadBuf[11] = digitalReadFast(11);
    gPinReadBuf[12] = digitalReadFast(12);
    pin = 14;
    gPinReadBuf[pin++] = digitalReadFast(14);
    gPinReadBuf[pin++] = digitalReadFast(15);
    gPinReadBuf[pin++] = digitalReadFast(16);
    gPinReadBuf[pin++] = digitalReadFast(17);
    gPinReadBuf[pin++] = digitalReadFast(18);
    gPinReadBuf[pin++] = digitalReadFast(19);
    gPinReadBuf[pin++] = digitalReadFast(20);

    gPinReadBuf[pin++] = digitalReadFast(21);
    gPinReadBuf[pin++] = digitalReadFast(22);
    gPinReadBuf[pin++] = digitalReadFast(23);
    gPinReadBuf[pin++] = digitalReadFast(24);
    gPinReadBuf[pin++] = digitalReadFast(25);
    gPinReadBuf[pin++] = digitalReadFast(26);
    gPinReadBuf[pin++] = digitalReadFast(27);
    gPinReadBuf[pin++] = digitalReadFast(28);
    gPinReadBuf[pin++] = digitalReadFast(29);
    gPinReadBuf[pin++] = digitalReadFast(30);

    gPinReadBuf[pin++] = digitalReadFast(31);
    gPinReadBuf[pin++] = digitalReadFast(32);
    gPinReadBuf[pin++] = digitalReadFast(33);
    gPinReadBuf[pin++] = digitalReadFast(34);
    gPinReadBuf[pin++] = digitalReadFast(35);
    gPinReadBuf[pin++] = digitalReadFast(36);
    gPinReadBuf[pin++] = digitalReadFast(37);
    gPinReadBuf[pin++] = digitalReadFast(38);
    gPinReadBuf[pin++] = digitalReadFast(39);
    gPinReadBuf[pin++] = digitalReadFast(40);

    gPinReadBuf[pin++] = digitalReadFast(41);
    gPinReadBuf[pin++] = digitalReadFast(42);
    gPinReadBuf[pin++] = digitalReadFast(43);
    gPinReadBuf[pin++] = digitalReadFast(44);
    gPinReadBuf[pin++] = digitalReadFast(45);
    gPinReadBuf[pin++] = digitalReadFast(46);
    gPinReadBuf[pin++] = digitalReadFast(47);
    gPinReadBuf[pin++] = digitalReadFast(48);
    gPinReadBuf[pin++] = digitalReadFast(49);
    gPinReadBuf[pin++] = digitalReadFast(50);

    gPinReadBuf[pin++] = digitalReadFast(51);
    gPinReadBuf[pin++] = digitalReadFast(52);
    gPinReadBuf[pin]   = digitalReadFast(53);
}

void readAllMega2560AnalogInputPins() {
    int pin = 54;

    gPinReadBuf[pin++] = digitalReadFast(54);
    gPinReadBuf[pin++] = digitalReadFast(55);
    gPinReadBuf[pin++] = digitalReadFast(56);
    gPinReadBuf[pin++] = digitalReadFast(57);
    gPinReadBuf[pin++] = digitalReadFast(58);
    gPinReadBuf[pin++] = digitalReadFast(59);
    gPinReadBuf[pin++] = digitalReadFast(60);
    gPinReadBuf[pin++] = digitalReadFast(61);
    gPinReadBuf[pin++] = digitalReadFast(62);
    gPinReadBuf[pin++] = digitalReadFast(63);
    gPinReadBuf[pin++] = digitalReadFast(64);
    gPinReadBuf[pin++] = digitalReadFast(65);
    gPinReadBuf[pin++] = digitalReadFast(66);
    gPinReadBuf[pin++] = digitalReadFast(67);
    gPinReadBuf[pin++] = digitalReadFast(68);
    gPinReadBuf[pin]   = digitalReadFast(69);
}
#endif

