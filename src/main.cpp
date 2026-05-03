#include <Arduino.h>
#include <Ethernet3.h>
#include <ListLib.h>
#include <MemoryFree.h>
#include <MIDI.h>
#include <AppleMIDI.h>

// USE_EXT_CALLBACKS is necessary to catch any MIDI events
// Since it only takes about 150 bytes of flash we turn it on by default.
#define USE_EXT_CALLBACKS
#include <AppleMIDI.h>

#define GEN_GLOBALS
#include "glob_gen.h"
#include "stringify.h"
#include "config_features.h"
#include "fastread.h"
#include "debug.h"
#include "data.h"
#include "midi_const.h"
#include "debouncer.h"
#include "pin_list.h"

/*
-------------------------------------------------------------------------------------
MIDI input scanner and remapping MIDI merger for Arduino and Teensy family processors
-------------------------------------------------------------------------------------

This is an organ oriented MIDI scanner whose primary function is to read digital
and analog input from various devices (keyboards, pedalboards, expression
pedals) and transmit corresponding MIDI messages to a computer or other
upstream MIDI device.

The implementation uses the well known 47Effects Arduino MIDI library, which
supports multiple transport interfaces including MIDI 31kbps serial (DIN-5 plugs),
USB-MIDI and Ethernet MIDI (RTP-MIDI / Apple Midi).

This package supports any combination of diode matrix and parallel input arrays up to
a total of 64-65 inputs.  This could allow as many as 7 8x8 diode matrix arrays
on the more powerful processors.
*/

// scan component speed test routines

void test_fastread() {
    for (int i = 2; i < 69; i++) {
        fastread(i);
    }
}

void test_diodeMatrix_8x8(byte *buf) {
    for (int colPin = 20; colPin < 28; colPin++) {
        fastwrite(colPin, LOW);
        int indx = 0;
        for (int readPin = 28; readPin < 36; readPin++) {
            buf[indx++] = fastread(readPin);
        }
        fastwrite(colPin, HIGH);
    }
}


// Minimal-RAM scan block processor based on flash PinBlock defs and separately allocated debouncers
// If we disavow the slow 16 MHz Arduinos this can be made more straightforward by building a fully RAM
// resident table when the flash pinBlock defs are read and debouncers are allocated.
void scanPinBlocks() {
    for (int i = 0; i < nPinBlocks; i++) {
        // copying the PinBlock struct into RAM is almost twice as slow as reading directly from flash; don't do it
        const PinBlock_t * pb = &(gPinBlocks[i]);
        midi::DataByte noteNum = pb->baseMidiNoteNum;
        int dbIndx = gDebouncerBases[i];

        // These help confirm that the PinBlock is being read and processed correctly
#if 0
        Console_print("cbase "); Console_println(pb->selectBasePin);
        Console_print("clim "); Console_println(pb->selectBasePin + pb->numSelectPins);
        Console_print("rbase "); Console_println(pb->readBasePin);
        Console_print("rlim "); Console_println(pb->readBasePin + pb->numReadPins);
#endif
        // process the pin block
        if (pb->useSelect) {
            // diode matrix - read pins loop inside of select pins loop
            midi::DataByte noteLim = pb->baseMidiNoteNum + pb->numCtrls;        // in case matrix has a non-full select row
            int clim = pb->selectBasePin + pb->numSelectPins;
            int rlim = pb->readBasePin + pb->numReadPins;
            for (int selPin = pb->selectBasePin; selPin < clim; selPin++) {
                //digitalWrite(selPin, pb->activeLow ? LOW : HIGH);  // activate the select pin
                fastwrite(selPin, pb->activeLow ? LOW : HIGH);

                // scan the read pins
                for (int readPin = pb->readBasePin; (readPin < rlim) && (noteNum < noteLim); readPin++) {
                    //int inp = digitalRead(rowPin);
                    int inp = fastread(readPin);

                    //gDebouncers[dbIndx++].stateSampleDummy(pb->activeLow ? !inp : inp, noteNum++, pb->midiOutChan); // logs notes and chans sequence
                    //gDebouncers[dbIndx++].stateSample(true, noteNum++, pb->midiOutChan);  // causes initial burst of noteOn for all notes
                    //gDebouncers[dbIndx++].stateSample(false, noteNum++, pb->midiOutChan);  // make sure it never sees anything

                    gDebouncers[dbIndx++].stateSample(pb->activeLow ? !inp : inp, noteNum++, pb->midiOutChan);
                }
                //digitalWrite(colPin, pb->activeLow ? HIGH : LOW);  // deactivate the select pin
                fastwrite(selPin, pb->activeLow ? HIGH : LOW);  // deactivate the select pin
            }
        }
        else {
            // parallel non-matrix inputs - single loop on read pins
            int rlim = pb->readBasePin + pb->numReadPins;
            for (int readPin = pb->readBasePin; readPin < rlim; readPin++) {
                int inp = fastread(readPin);
                gDebouncers[dbIndx++].stateSample(pb->activeLow ? !inp : inp, noteNum++, pb->midiOutChan);
            }
        }

    }
}

// remapping callbacks
// We wouldn't need a separate handler per serial interface if we could introspect 
// the port but there is no context pointer given to the callbacks (oops).
#if DO_CHANNEL_SEPARATION_REMAP
void noteOffHandlerMidi0(midi::Channel channel, byte notenum, byte velocity) {
    uint8_t outChan = remapMidiChans[0] ? midiRemappedChans[0] : channel;
    midi0.sendNoteOff(notenum, velocity, outChan);
}
void noteOnHandlerMidi0(midi::Channel channel, byte notenum, byte velocity) {
    uint8_t outChan = remapMidiChans[0] ? midiRemappedChans[0] : channel;
    midi0.sendNoteOn(notenum, velocity, outChan);
}
void aftertouchPolyHandlerMidi0(midi::Channel channel, byte notenum, byte pressure) {
    uint8_t outChan = remapMidiChans[0] ? midiRemappedChans[0] : channel;
    midi0.sendAfterTouch(notenum, pressure, outChan);
}
void controlChangeHandlerMidi0(midi::Channel channel, byte cnum, byte cval) {
    uint8_t outChan = remapMidiChans[0] ? midiRemappedChans[0] : channel;
    midi0.sendControlChange(cnum, cval, outChan);
}
void programChangeHandlerMidi0(midi::Channel channel, byte pgm) {
    uint8_t outChan = remapMidiChans[0] ? midiRemappedChans[0] : channel;
    midi0.sendProgramChange(pgm, outChan);    
}
void aftertouchChannelHandlerMidi0(midi::Channel channel, byte pressure) {
    uint8_t outChan = remapMidiChans[0] ? midiRemappedChans[0] : channel;
    midi0.sendAfterTouch(pressure, outChan);
}
void pitchBendHandlerMidi0(midi::Channel channel, int pitchval) {
    uint8_t outChan = remapMidiChans[0] ? midiRemappedChans[0] : channel;
    midi0.sendPitchBend(pitchval, outChan);
}
#endif

// Start all MIDI ports and configure them
//
// For serial MIDI ports, this is where we set up channel separation remapping by setting callbacks
// on all of the channel msg types.
//
// Possibly useful oddity:
// If you start a MIDI port with OMNI channels and leave THRU on (the default), everything will be sent onward.
// However, if you then set the thru filter mode to "other channels" via setThruFilterMode(DifferentChannel),
// it has the curious effect of discarding all channel messages while sending onward all non-channel msgs.
//
void startMidi()
{
#if MERGE_SERIAL_INPUTS
#if MIDI_MERGE_SERIAL_PORTS > 0
    // Merge with optional remapping
    // Remapping via the callbacks is a bit tedious but the callbacks have varying signatures
    // Need separate handlers for each MIDI port since you can't introspect the port from inside the callback
    midi0.begin(MIDI_CHANNEL_OMNI);

#if DO_CHANNEL_SEPARATION_REMAP
    midi0.setHandleNoteOff(noteOffHandlerMidi0);
    midi0.setHandleNoteOn(noteOnHandlerMidi0);
    midi0.setHandleAfterTouchPoly(aftertouchPolyHandlerMidi0);
    midi0.setHandleControlChange(controlChangeHandlerMidi0);
    midi0.setHandleProgramChange(programChangeHandlerMidi0);
    midi0.setHandleAfterTouchChannel(aftertouchChannelHandlerMidi0);    
#endif

    // Kill channel msgs, pass on all non-chanel msgs
    // midi0.begin(MIDI_CHANNEL_OMNI);
    // midi0.setThruFilterMode(midi::Thru::DifferentChannel); // would only send chanmsg thru if (!omni && !omni) => never

    // Decoder-only setup that is a sink of messages on a specific channel and does no merge
    //midi0.begin(decodeChan);
    //midi0.turnThruOff();
#endif // MIDI_MERGE_SERIAL_PORTS > 0

#if MIDI_MERGE_SERIAL_PORTS > 1
    midi1.begin(MIDI_CHANNEL_OMNI);
#endif

#if MIDI_MERGE_SERIAL_PORTS > 2
    midi2.begin(MIDI_CHANNEL_OMNI);
#endif
#endif // MERGE_SERIAL_INPUTS

#if ETHERNET_MIDI_CONNECT
    AM_DBG_SETUP(consoleBaudRate);       // *** may collide with our debug setup?

    // Ethernet interface setup before AppleMidi    
    Ethernet.init(4);   // set sockets to 4, giving larger 4k buffers
    // form unique hostname using the macaddr
    // hostname limit is generally 64 bytes in a single label (fqdn can be 253)
    // make "hostname-XXxxXXxxXXxx" with 6 octets of mac addr so max 78 chars + the null
    // This name will be unique among all official Arduino Ethernet shields ever made with baked-in mac addrs
    char buf[79];
    snprintf(buf, sizeof(buf), ETH_HOSTNAME_PREFIX "-%2X%2X%2X%2X%2X%2X",
        gEthernetMac[0], gEthernetMac[1], gEthernetMac[2],
        gEthernetMac[3], gEthernetMac[4], gEthernetMac[5]);
    Ethernet.setHostname(buf);
    // Get IP via DHCP.  We cannot continue if this fails.
    while (Ethernet.begin(gEthernetMac) == 0) {
        Console_println(F("Failed DHCP, retrying"));
        delay(500);
    }
    AM_DBG(F("DHCP Success.  Host params:"));
    AM_DBG(F(" hostname: "), buf);
    AM_DBG(F(" IP      : "), Ethernet.localIP());

    // make session name globally (and I do mean globally) unique by changing the default prefix and appending the full macaddr
    // I don't know if we will want to provide multiple sessions; if so they will need to be uniqified with a session counter.
    snprintf(buf, sizeof(buf), "Apple-dbcproc-%2X%2X%2X%2X%2X%2X",
        gEthernetMac[0], gEthernetMac[1], gEthernetMac[2], 
        gEthernetMac[3], gEthernetMac[4], gEthernetMac[5]);

    // The following create a MIDI interface object called "ETHMIDI" which has an AppleMidi session object as its transport.
    // The AppleMidi session runs on the default port 5004, with a session name that we specify, running EthernetUDP
    // The name of the MIDI instance var is directly specified by the 2nd parameter to APPLEMIDI_CREATE_INSTANCE.
    // The name of the AppleMidi instance var is Apple##foo where foo is param 2 to the create-instance call.
    //
    // The varnames are both based on the 2nd parameter to APPLEMIDI_CREATE_INSTANCE.  This basename MUST be a
    // compile-time literal.  There is no way to make it otherwise given how the libraries work; these objects have to
    // reside at specific locations once created since interrupt-driven callbacks are in play.  It's OK to take their
    // addresses, but if you copy the objects to another location, things will quit working and you will have an
    // interesting debug session figuring out why.
    // Therefore we must use compile-time programming to set the varnames if we want multiple sessions.
    //
    // We override the default basename of "MIDI" so that we can make unique across all devices, which aids
    // discovery by the client software.
    //
    
    APPLEMIDI_CREATE_INSTANCE(EthernetUDP, ETHMIDI, buf, DEFAULT_CONTROL_PORT);

    AM_DBG(F("AppleMIDI UDP session started.  Params:"));
    AM_DBG(F(" Name: "), AppleMIDI.getName());
    AM_DBG(F(" Port: "), AppleMIDI.getPort());

    // capture the interface in MIDI lib land
    //gMidiEthOutputInterface = &ETHMIDI;
    gMidiEthOutputInterface = &ETH_MIDI_BASENAME;
    gMidiEthOutputInterface->begin(ETH_MIDI_LISTEN_CHAN);

    // capture the AppleMidi instance needed to manipulate callbacks etc.
    gAppleMidiInstance = &AppleETHMIDI;
    
    // Set up callbacks to monitor AppleMidi connections

    // connect/disconnect are the standard callbacks available without USE_EXT_CALLBACKS
    gAppleMidiInstance->setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
        gEthConnections++;
        AM_DBG(F("Eth connect "), ssrc, name, F(" NSessions "), gEthConnections);
    });
    gAppleMidiInstance->setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
        gEthConnections--;
        AM_DBG(F("Eth disconnect "), ssrc, F(" NSessions "), gEthConnections);
    });

    // for scanner/encoder functions, the main scanner routine will just need to call
    // gMidiEthOutputInterface->sendNoteOn(note, vel, channel) in concert with the debouncers.
    // The debouncer will have to be given enough information to know what transport to send over.
    // This has already been a RAM consumption issue for the Arduino Mega.
    
    // for decoder functions we'll need to add callbacks for the messages we accept
#if ETHERNET_MIDI_DECODE_INPUT
    gMidiEthOutputInterface->setHandleNoteOn([](byte channel, byte note, byte velocity) {
        AM_DBG(F("NoteOn"), channel, note, velocity);
    });
    gMidiEthOutputInterface->setHandleNoteOff([](byte channel, byte note, byte velocity) {
        AM_DBG(F("NoteOff"), channel, note, velocity);
    });
    gMidiEthOutputInterface->setHandleControlChange([](Channel channel, byte v1, byte v2) {
        AM_DBG(F("ControlChange"), channel, v1, v2);
    });
#endif
#if 0
    // The remaining basic channel message callbacks are these:
    MIDI.setHandleProgramChange([](Channel channel, byte v1) {
        AM_DBG(F("ProgramChange"), channel, v1);
    });
    MIDI.setHandlePitchBend([](Channel channel, int v1) {
        AM_DBG(F("PitchBend"), channel, v1);
    });
#endif

#endif // ETHERNET_MIDI_CONNECT

}


void configurePins() {
    for (int i = 0; i < nPinBlocks; i++) {
        const PinBlock_t * pb = &gPinBlocks[i];

        if (pb->useSelect) {
            for (uint8_t selPin = pb->selectBasePin; selPin < pb->selectBasePin + pb->numSelectPins; selPin++) {
                pinMode(selPin, OUTPUT);
                digitalWrite(selPin, pb->activeLow ? HIGH : LOW);
            }
        }
        for (uint8_t readPin = pb->readBasePin; readPin < pb->readBasePin + pb->numReadPins; readPin++) {
            pinMode(readPin, pb->activeLow ? INPUT_PULLUP : INPUT);
        }
    }

}


void setup() 
{
    pinMode(LED_BUILTIN, OUTPUT);

#if USE_DEBUG_PRINT
    Console->begin(consoleBaudRate);
#endif

    initDebouncers();
    initDebouncerBases();

    configurePins();
    
    startMidi();
}

// statics for the main loop
unsigned long lastMillis = 0;
unsigned long loopCount = 0;

void loop() 
{
    // loop rate tracking data for Arduino Mega 2560
    //   For reasons yet to be determined, diode matrix scanning time is a little nonlinear vs matrix size.
    //   It seems like the higher number IO pins may be slower.
    //   All of these numbers are BEFORE the implementation of fastread - they will improve considerably
    //    raw rate with nothing but rate tracking:             165.7 KHz +/- 0.1 KHz
    //    raw rate with a single function call                 117.0 KHz
    //    single 8x4 matrix block                                2.64 KHz (379 usec = 11.8 usec per input)
    //    single 8x8 matrix block                                1.56 KHz (pins 16-31 = 10.0 usec per input)  1.44 KHz (pins 32-48 = 10.8 usec per input)
    //    two 8x8 matrix blocks                                  0.641 KHz (1560 usec = 12.2 usec per input )
    //    single block of 32 parallels                           3.48 KHz (287 usec = 8.97 usec per input)
    loopCount++;
    unsigned long curMillis = millis();
    if (curMillis - lastMillis > 1000) {
        lastMillis = curMillis;
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        Console_print(F("rate: ")); Console_println(loopCount);
        Console_print(F("memfree: ")); Console_println(freeMemory());
        loopCount = 0;
    }

    //test_fastread();
    // byte buf[10];
    //test_diodeMatrix_8x8(buf);
    scanPinBlocks();
}


