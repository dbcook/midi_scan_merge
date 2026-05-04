#pragma once

#include <Arduino.h>
#if defined(ARDUINO_AVR_MEGA2560)
#include <EEPROM.h>
#elif defined(ARDUINO_SAM_DUE)
#include <DueFlashStorage.h>
#error Due flash not supported yet
#endif

#include "glob_gen.h"


// *** This file can currently only be used for Arduino Mega 2560 which has 4K built-in flash
// *** Arduino Due does not have built-in flash (SAM processor vs AVR) and so you have to use
// *** the Due Flash Storage Library https://github.com/sebnil/DueFlashStorage
// *** The Due storage library is also not backward-compatible with the Megas because the latter
// *** cannot easily write their regular flash outside the bootloader.
// Note that the detection symbol for the Due is mistakenly associated with the AVR architecture.

#if defined(ARDUINO_AVR_MEGA2560)
#elif defined(ARDUINO_SAM_DUE)  // Arduino Due
#endif



#define SERNO_LEN   13    // 12 bytes plus null

class NvMem {

    protected:
        // non-volatile param storage layout
        // User level EEPROM addressing starts at 0; the lib routines add offsets to the correct memory space
        typedef struct {
            uint16_t signature;                         // DEADBEEF if our nv storage has been initialized
            byte macaddr[6];                            // Ethernet MAC address
            byte serno[SERNO_LEN];                      // board serial number (12-chars) stored as string
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
            uint16_t sig = eeprom_read_word(addr);
            if (sig != 0xDEADBEEF) initialized = false;
            if (force || !initialized) {
                // init with a randomized locally administered macaddr
                // should almost always result in a unique address (collision chance 1e-9)
                uint32_t rn = random();
                byte newmac[6] = { 0x02, 0xBE, rn & 0x000000FF, (rn & 0x0000FF00) >> 8, (rn & 0x00FF0000) >> 16, (rn & 0xFF000000) >> 24 };
                updateMacAddr(newmac);
            }
        }
        static void getSerNo(char * buf) {
            char * nvaddr = (char *)offsetof(nv_mem_layout, serno);
            eeprom_read_block(buf, nvaddr, SERNO_LEN);
        }

        static void updateSerNo(const char * buf) {
            char * nvaddr = (char *)offsetof(nv_mem_layout, serno);
            eeprom_update_block(buf, nvaddr, SERNO_LEN);
        }

        static void getMacAddr(byte * buf) {
            byte * nvaddr = (byte *)offsetof(nv_mem_layout, macaddr);
            eeprom_read_block(buf, nvaddr, 6);
        }

        static void updateMacAddr(byte * buf) {
            byte * nvaddr = (byte *)offsetof(nv_mem_layout, macaddr);
            eeprom_update_block(buf, nvaddr, 6);
        }

        static uint32_t getDefaultDebounceMsec() {
            uint32_t * nvaddr = (uint32_t *)offsetof(nv_mem_layout, default_debounce_msec);
            return eeprom_read_dword(nvaddr);
        }

        static void updateDefaultDebounceMsec(uint32_t msec) {
            uint32_t * nvaddr = (uint32_t *)offsetof(nv_mem_layout, default_debounce_msec);
            eeprom_update_dword(nvaddr, msec);
        }

};