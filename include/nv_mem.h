#pragma once

#include <Arduino.h>

// EEPROM.h can currently only be used for Arduino Mega 2560 which has 4K built-in EEPROM.
// Arduino Due does not have built-in flash (SAM processor vs AVR) and so you have to use
// the Due Flash Storage Library https://github.com/sebnil/DueFlashStorage
// The Due storage library is also not backward-compatible with the Megas because the latter
// cannot easily write their regular flash outside the bootloader.
// The Due library is very primitive and does not even support a multi-byte read
// *** Fork DueFlashStorage and add
// ***  word, long, and arbitrary length read
// ***  word, long writes (requires 4-byte boundaries)
// ***  update versions of all writes (compare with existing flash before writing)

#if defined(ARDUINO_AVR_MEGA2560)
#include <EEPROM.h>
#elif defined(ARDUINO_SAM_DUE)
#include <DueFlashStorage.h>
DueFlashStorage DueFlash;   // static instance needed
#endif

#include "glob_gen.h"

// *** Look at using the SD card on the Ethernet Shield 2 to store params
// *** that should be completely portable.  The code here will be useful for
// *** cases where Ethernet is not needed.

// macros to unify comparable NV access calls.  arg order is congruent with EEPROM.h
// the SAM macos need enhancements to the DueFlashStorage library which lacks many convenience calls.
#if defined(ARDUINO_AVR_MEGA2560)
#define NV_READ_BYTE(offset) eeprom_read_byte(offset)
#define NV_READ_WORD(offset) eeprom_read_word(offset)
#define NV_READ_DWORD(offset) eeprom_read_dword(offset)
#define NV_READ_BLOCK(buf, offset, len) eeprom_read_block(buf, offset, len)
#define NV_WRITE_BYTE(offset, val) eeprom_write_byte(offset, val)
#define NV_WRITE_WORD(offset, val) eeprom_write_word(offset, val)
#define NV_WRITE_DWORD(offset, val) eeprom_write_dword(offset, val)
#define NV_WRITE_BLOCK(buf, offset, len) eeprom_write_block(buf, offset, len)
#define NV_UPDATE_BYTE(offset, val) eeprom_update_byte(offset, val)
#define NV_UPDATE_WORD(offset, val) eeprom_update_word(offset, val)
#define NV_UPDATE_DWORD(offset, val) eeprom_update_dword(offset, val)
#define NV_UPDATE_BLOCK(buf, offset, len) eeprom_update_block(buf, offset, len)
#elif defined(ARDUINO_SAM_DUE)  // Arduino Due
#define NV_READ_BYTE(offset) DueFlashStorage.read(offset)
#define NV_READ_WORD(offset)    0  // *** empty
#define NV_READ_DWORD(offset)   0 // *** empty
#define NV_READ_BLOCK(buf, offset, len) nullptr   // *** empty
#define NV_WRITE_BYTE(offset, val) DueFlashStorage.write(offset, val)
#define NV_WRITE_WORD(offset, val) nullptr  // *** empty
#define NV_WRITE_DWORD(offset, val) nullptr  // *** empty
#define NV_WRITE_BLOCK(buf, offset, len) DueFlashStorage.write(addr, offset, len)
#define NV_UPDATE_BYTE(offset, val) nullptr
#define NV_UPDATE_WORD(offset, val) nullptr
#define NV_UPDATE_DWORD(offset, val) nullptr
#define NV_UPDATE_BLOCK(buf, offset, len) nullptr
#endif



#define SERNO_LEN   13    // 12 bytes plus null

class NvMem {

    protected:
        // non-volatile param storage layout
        // User level EEPROM addressing starts at 0; the lib routines add offsets to the correct memory space
        // For Due flash NV, all multi-byte items must start on a 4-byte boundary
        typedef struct {
            uint16_t signature;                         // DEADBEEF if our nv storage has been initialized
            byte macaddr[6];                            // Ethernet MAC address
            byte align1[2];
            byte serno[SERNO_LEN];                      // board serial number (12-chars) stored as string
            byte align2[3];
            uint32_t default_debounce_msec;             // default contact debounce time in milliseconds
            byte single_contact_default_note_on_vel;    // default note-on velocity for single-contact keyboards
            byte single_contact_default_note_off_vel;   // default note-off velocity for single-contact keyboards
        } nv_mem_layout;

    public:

        // factory defaults the nv storage *only* if bad signature or force flag is used
        // therefore safe to call at every boot
        static void initNvMem(bool force = false) {
            bool initialized = true;
            uint16_t * addr = (uint16_t *)offsetof(nv_mem_layout, signature);
            uint16_t sig;
            sig = NV_READ_WORD(addr);
            if (sig != 0xDEADBEEF) initialized = false;
            if (force || !initialized) {
                // init with a randomized locally administered macaddr
                // should almost always result in a unique address (collision chance 1e-9)
                uint32_t rn = random();
                byte newmac[6] = { 0x02, 0xBE, (byte)(rn & 0x000000FF), (byte)((rn & 0x0000FF00) >> 8),
                    (byte)((rn & 0x00FF0000) >> 16), (byte)((rn & 0xFF000000) >> 24) };
                updateMacAddr(newmac);
            }
        }
        static void getSerNo(char * buf) {
            char * nvaddr = (char *)offsetof(nv_mem_layout, serno);
            NV_READ_BLOCK(buf, nvaddr, SERNO_LEN);
        }

        static void updateSerNo(const char * buf) {
            char * nvaddr = (char *)offsetof(nv_mem_layout, serno);
#if defined(ARDUINO_AVR_MEGA2560)
            eeprom_update_block(buf, nvaddr, SERNO_LEN);
#elif defined(ARDUINO_SAM_DUE)
#endif
        }

        static void getMacAddr(byte * buf) {
            byte * nvaddr = (byte *)offsetof(nv_mem_layout, macaddr);
            NV_READ_BLOCK(buf, nvaddr, 6);
        }

        static void updateMacAddr(byte * buf) {
            byte * nvaddr = (byte *)offsetof(nv_mem_layout, macaddr);
#if defined(ARDUINO_AVR_MEGA2560)
            eeprom_update_block(buf, nvaddr, 6);
#elif defined(ARDUINO_SAM_DUE)
#endif
        }

        static uint32_t getDefaultDebounceMsec() {
            uint32_t * nvaddr = (uint32_t *)offsetof(nv_mem_layout, default_debounce_msec);
            return NV_READ_DWORD(nvaddr);
        }

        static void updateDefaultDebounceMsec(uint32_t msec) {
            uint32_t * nvaddr = (uint32_t *)offsetof(nv_mem_layout, default_debounce_msec);
#if defined(ARDUINO_AVR_MEGA2560)
            eeprom_update_dword(nvaddr, msec);
#elif defined(ARDUINO_SAM_DUE)
#endif
        }

};