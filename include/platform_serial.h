#pragma once

#include <Arduino.h>
#include <MIDI.h>
#include "glob_gen.h"
#include "ArrayList.h"
#include "serial_gcm4.h"


// Serial port constraints
// The MIDI serial output port, if used, cannot be the same as any serial merge input channel
// The MIDI library port names have to be compile time literals (not even #define's) and the macros have to be at global scope

// Create a MIDI lib interface object for all serial ports that you might want to use
// Once the library port object is created you can take the address of the object and stash it.
// As long as you don't call xxxx.begin() it will not mess with the serial port or claim the pins.

// Nail this down for simplicity - four hardware UARTs are available on all platforms of interest
const size_t numHardwareUartPorts = 4;

// This bit of platform dependence is needed anywhere we need the Arduino serial port class names
#ifdef ARDUINO_SAM_DUE
#define PLATFORM_SERCLASS HardwareSerial
#elif defined( ARDUINO_GRAND_CENTRAL_M4 )
// In GC M4, Serial addresses the native USB port and has type Serial_
// For other hardware serial ports Serial1, ... have class type Uart
#define PLATFORM_SERCLASS_NATIVE_USB Serial_
#define PLATFORM_SERCLASS Uart
#endif

typedef MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<PLATFORM_SERCLASS>> * pMidiInterface_t;

// Statically create MIDI interface class objects and bind them to hardware serial ports

// We can take the addresses of midi0... and store those pointers so we don't have to use the literal
// names in subsequent code.  A mapping will be used to look up what functions are assigned to particular
// UARTs.
//
// ORDERING: the order of UART to MIDI interface definitions is according to increasing pin numbers.
//  Ports will be allocated in order of midiN, which guarantees
// that the first port allocated will have the smallest pin numbers and thus leave as many contiguous pins as possible.


#if defined( GEN_GLOBALS )

#ifdef ARDUINO_SAM_DUE
MIDI_CREATE_INSTANCE(PLATFORM_SERCLASS, Serial, midi0);
MIDI_CREATE_INSTANCE(PLATFORM_SERCLASS, Serial3, midi1);
MIDI_CREATE_INSTANCE(PLATFORM_SERCLASS, Serial2, midi2);
MIDI_CREATE_INSTANCE(PLATFORM_SERCLASS, Serial1, midi3);
#elif defined( ARDUINO_GRAND_CENTRAL_M4 )
MIDI_CREATE_INSTANCE(PLATFORM_SERCLASS, Serial1, midi1);
MIDI_CREATE_INSTANCE(PLATFORM_SERCLASS, Serial4, midi2);
MIDI_CREATE_INSTANCE(PLATFORM_SERCLASS, Serial3, midi3);
MIDI_CREATE_INSTANCE(PLATFORM_SERCLASS, Serial2, midi4);    // this one might not work on Due
#else
#error Undefined board type!
#endif

#endif
EXTERN pMidiInterface_t pMidi0
#if defined( GEN_GLOBALS )
    = &midi0
#endif
;
EXTERN pMidiInterface_t pMidi1
#if defined( GEN_GLOBALS )
    = &midi1
#endif
;
EXTERN pMidiInterface_t pMidi2
#if defined( GEN_GLOBALS )
    = &midi2
#endif
;
EXTERN pMidiInterface_t pMidi3
#if defined( GEN_GLOBALS )
    = &midi3
#endif
;

// Serial MIDI interface allocator / tracker

class UartToMidiInterfaceMap {
    public:

        typedef enum {
            unassigned,
            midi_serial_output,                 // MIDI output transport
            midi_serial_input,                  // input that will be thru-forwarded to MIDI output transports
            midi_serial_input_chan_remap        // input forwarded with channel remapping
        } eAllocatedFunction;

        class SerMidiMapEntry {
            public:
                PLATFORM_SERCLASS * pUart;          // The serial object, e.g. Serial, Serial1, etc.
                pMidiInterface_t pMidiInterface;    // The MIDI interface object associated to the UART
                eAllocatedFunction function;

                SerMidiMapEntry() {
                    function = unassigned;
                }
                SerMidiMapEntry(PLATFORM_SERCLASS * pUart, pMidiInterface_t pMidiInterface, eAllocatedFunction funct = unassigned) {
                    this->pUart = pUart;
                    this->pMidiInterface = pMidiInterface;
                    this->function = funct;
                }
        };

        ArrayList<SerMidiMapEntry> midiMapList;
        SerMidiMapEntry * pOutputInterface = NULL;
        SerMidiMapEntry * pInputInterfaces[numHardwareUartPorts] = {NULL, NULL, NULL, NULL};
        uint8_t numInputInterfaces = 0;

        UartToMidiInterfaceMap() {
#if defined( ARDUINO_GRAND_CENTRAL_M4 )

            midiMapList.add(SerMidiMapEntry(&Serial1, pMidi0));
            midiMapList.add(SerMidiMapEntry(&Serial4, pMidi1));
            midiMapList.add(SerMidiMapEntry(&Serial3, pMidi2));
            midiMapList.add(SerMidiMapEntry(&Serial2, pMidi3));

#elif defined( ARDUINO_SAM_DUE )

            midiMapList.add(SerMidiMapEntry(&Serial, pMidi0));
            midiMapList.add(SerMidiMapEntry(&Serial3, pMidi1));
            midiMapList.add(SerMidiMapEntry(&Serial2, pMidi2));
            midiMapList.add(SerMidiMapEntry(&Serial1, pMidi3));

#else
#error Unimplemented board type!
#endif
        }

        bool allocateNextFreePort( eAllocatedFunction function) {
            for (auto pmap = midiMapList.begin(); pmap != midiMapList.end(); pmap++) {
                if (pmap->function == eAllocatedFunction::unassigned) {
                    pmap->function = function;
                    if (function == eAllocatedFunction::midi_serial_output) {
                        pOutputInterface = &*pmap;
                    }
                    else if ((function == eAllocatedFunction::midi_serial_input) || function == eAllocatedFunction::midi_serial_input_chan_remap ) {
                        pInputInterfaces[numInputInterfaces++] = &*pmap;
                    }
                    return true;
                }
            }
        }

        void startAllocatedPorts() {
            for (auto pmap = midiMapList.begin(); pmap != midiMapList.end(); pmap++) {
                if (pmap->function != eAllocatedFunction::unassigned) {
                    pmap->pMidiInterface->begin();
                }
            }
        }

        pMidiInterface_t getOutputInterface() {
            return pOutputInterface->pMidiInterface;
        }

        uint8_t getNumInputInterfaces() {
            return numInputInterfaces;
        }

        pMidiInterface_t getInputInterface(size_t i) {
            if (i < numInputInterfaces) {
                return pInputInterfaces[i]->pMidiInterface;
            }
            return NULL;
        }

        // tell us whether a given input interface is regular or remapped
        eAllocatedFunction getInputInterfaceFunction(size_t i) {
            if (i < numInputInterfaces) {
                return pInputInterfaces[i]->function;
            }
            return eAllocatedFunction::unassigned;
        }

} ;
