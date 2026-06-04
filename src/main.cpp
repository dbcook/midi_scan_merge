/*
-------------------------------------------------------------------------------------
MIDI scanner / merger/ decodeer for Arduino and Teensy family processors
-------------------------------------------------------------------------------------

This is a multi-function organ oriented MIDI scanner / merger / decoder for Arduino
architecture processors such as the AdaFruit Grand Central M4, Arduino Due, and Teensy. Its primary functions are:

1) Read digital and analog input from various devices (keyboards, pedalboards, expression
pedals) and transmit corresponding MIDI messages to a computer or other
upstream MIDI device via Ethernet, USB or classic MIDI serial transports.

2) Provide MIDI scan/merge capability, where up to 3 incoming classic MIDI serial
ports can be merged onto the output transport along with scanned signals from the hardware inputs.

3) [Future feature] Decode MIDI messages coming from upstream to operate hardware signal outputs.
These outputs could be LED drivers, solenoid relays, or even a display.

The implementation uses the well known 47Effects Arduino MIDI library, which
supports multiple add-on transport interfaces including MIDI 31kbps serial (DIN-5 plugs),
USB-MIDI and Ethernet MIDI (RTP-MIDI / Apple Midi).

The scanner features in this package support any combination of diode matrix and parallel input arrays up to
a total of 64-65 physical inputs.  This could allow as many as 7 8x8 diode matrix arrays on the more
powerful processors.
*/

#include <Arduino.h>
#include <Ethernet3.h>
#include <ListLib.h>
#include <MemoryFree.h>

// Due: This brings ignorable compile warnings stemming from -Wreorder in Wire.h - members initialized in ctor in different order than declared.
// Apparently only affects SAM processors and not SAMD, we get no warnings on Grand Central.
#include<LiquidCrystal_I2C.h>

#include <MIDI.h>
#include <AppleMIDI.h>

#define GEN_GLOBALS
#include "glob_gen.h"
#include "lcd_display.h"
#include "debug.h"
#include "stringify.h"
#include "config_features.h"
#include "fastread.h"
#include "debug.h"
#include "data.h"
#include "midi_const.h"
#include "debouncer.h"
#include "scanner.h"
#include "analog_lpf.h"

// MIDI channel remapping callbacks
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

void OnAppleMidiConnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
  gEthConnections++;
  AM_DBG(F("AppleMidi received connect to session"), ssrc, name);
}

void OnAppleMidiDisconnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
  gEthConnections--;
  AM_DBG(F("AppleMidi disconnect"), ssrc);
}


// Start MIDI ports on all enabled transports and configure them
//
// For serial MIDI ports, this is where we set up channel separation remapping by setting callbacks
// on all of the channel msg types.
//
// Possibly useful oddity:
// If you start a MIDI port with OMNI channels and leave THRU on (the default), everything will be sent onward.
// However, if you then set the thru filter mode to "other channels" via setThruFilterMode(DifferentChannel),
// it has the curious effect of discarding all channel messages while sending onward all non-channel msgs.
//

// Due to how APPLEMIDI_CREATE_INSTANCE works, the MIDI interface name (arg 2) has to be a pure literal (not stringized)
// and the session name (arg 3) has to be a compile time constant string.
// This produces a MIDI interface named MIDI and an AppleMidi session var named AppleMIDI, both based on the 2nd arg
// There is no way to switch this off with runtime config, so we have to accept the memory use.
APPLEMIDI_CREATE_INSTANCE(EthernetUDP, MIDI, "Ethernet_MIDI", DEFAULT_CONTROL_PORT);



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

    if (gConfig.useEthernet) {
        // Ethernet interface setup before AppleMidi    
        Ethernet.init(4);   // set sockets to 4, giving larger 4k buffers

        // form unique hostname using the macaddr
        // hostname limit is generally 64 bytes in a single label (fqdn can be 253)
        // make "hostname-XXxxXXxxXXxx" with 6 octets of mac addr so max 78 chars + the null
        // This name will be unique among all official Arduino Ethernet shields ever made with baked-in mac addrs
        char buf[79];
        snprintf(buf, sizeof(buf), ETH_HOSTNAME_PREFIX "-%02X%02X%02X%02X%02X%02X",
            gEthernetMac[0], gEthernetMac[1], gEthernetMac[2],
            gEthernetMac[3], gEthernetMac[4], gEthernetMac[5]);
        Ethernet.setHostname(buf);

        // Get IP via DHCP.  We cannot continue if this fails.
        while (Ethernet.begin(gEthernetMac) == 0) {
            Console_println(F("Failed DHCP, retrying"));
            delay(500);
        }
        
        AM_DBG(F("DHCP Success.  Host params:"));
        AM_DBG(F(" hostname:"), buf);
        AM_DBG(F(" IP      :"), Ethernet.localIP());

        // make session name globally (and I do mean globally) unique by changing the default prefix and appending the full macaddr
        // I don't know if we will want to provide multiple sessions; if so they need to be uniqified at compile time.

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

        AM_DBG(F("AppleMIDI UDP session starting.  Params:"));
        snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X",
            gEthernetMac[0], gEthernetMac[1], gEthernetMac[2],
            gEthernetMac[3], gEthernetMac[4], gEthernetMac[5]);
        AM_DBG(F(" Mac Addr: "), buf);

        AM_DBG(F(" AppleMidi Name:"), AppleMIDI.getName());
        AM_DBG(F(" Port:"), AppleMIDI.getPort());

        MIDI.begin();
        AM_DBG(F("AppleMIDI started"));
        
        // capture addr of the base MIDI interface in MIDI lib land
        gMidiEthOutputInterface = &ETH_MIDI_BASENAME;
        gMidiEthOutputInterface->begin(ETH_MIDI_LISTEN_CHAN);

        // capture the AppleMidi instance needed to manipulate callbacks etc.
        gAppleMidiInstance = &AppleMIDI;
        
        // Set up callbacks to monitor AppleMidi connections
        // connect/disconnect are the standard callbacks available without USE_EXT_CALLBACKS
        AppleMIDI.setHandleConnected(OnAppleMidiConnected);
        AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);

        // for scanner/encoder functions, the main scanner routine will just need to call
        // gMidiEthOutputInterface->sendNoteOn(note, vel, channel) in concert with the debouncers.
        // The debouncer has to be given enough information to know what transport to use.
        
        // for decoder functions we need callbacks for the messages we accept
    #if ETHERNET_MIDI_DECODE_INPUT
        gMidiEthOutputInterface->setHandleNoteOn([](byte channel, byte note, byte velocity) {
            AM_DBG(F("Decoded NoteOn"), channel, note, velocity);
        });
        gMidiEthOutputInterface->setHandleNoteOff([](byte channel, byte note, byte velocity) {
            AM_DBG(F("Decoded NoteOff"), channel, note, velocity);
        });
        gMidiEthOutputInterface->setHandleControlChange([](Channel channel, byte v1, byte v2) {
            AM_DBG(F("Decoded CC"), channel, v1, v2);
        });
    #endif
    #if 0
        // The remaining AppleMIDI basic channel message callbacks are these:
        MIDI.setHandleProgramChange([](Channel channel, byte v1) {
            AM_DBG("ProgramChange", channel, v1);
        });
        MIDI.setHandlePitchBend([](Channel channel, int v1) {
            AM_DBG("PitchBend"Í, channel, v1);
        });
    #endif

    }

} // startMidi


void configurePins() {
    pinMode(LED_BUILTIN, OUTPUT);
    // TODO check for illegal pins here
    InputScanner::checkPinFunctionConflicts();
    
    // configuring methods currently check for patently illegal pin assignments - factor that out and do above
    InputScanner::configureDigitaPins();
    InputScanner::configureAnalogPins();
}

// Startup for the optional 20x4 LCD display
// This is all harmless if the LCD is not connected as long as nothing else is on I2C address 0x27
// IMPORTANT: Any LCD messages to be displayed during normal MIDI scanning operation need to be done either in a background thread
// or at low frequency due to significant hard waits.  Crash messages can and should be done in the foreground.
// TODO make into an LCD handling class since we'll want a ringbuffer deque of message objects queued for the background thread
//

// NOTE: The I2C consumes 2 pins - in Arduino large-format boards these are digital 20-21.
void initLCD() {
    if (!gConfig.useLcd) return;
    gLcd = new LcdDisplay();
    gLcd->init();
    // Display initial startup banner
    gLcd->lcdMessage("DBCook MIDI ", 0, 0);
    gLcd->lcdMessage(gProdVersion);
}

void showStartupBannerOnLcd() {
    if (!gConfig.useLcd) return;
    // leave row 0 alone for init msg with version

    // Line 1: configuration - enabled features etc
    gLcd->pLCD->setCursor(0, 1);
    if (gConfig.useEthernet) {
        gLcd->lcdMessage("ETH ");
    }
    if (gConfig.useUSB) {
        gLcd->lcdMessage("USB ");
    }
    if (gUseSerialMidi) {
        gLcd->lcdMessage("SER ");
    }

    // Line 2: pin counts for various input types
    gLcd->lcdMessage("Dig ", 0, 2);
    gLcd->lcdMessage(String(gPinBlocksDigital.size()).c_str());
    gLcd->lcdMessage(" ");
    gLcd->lcdMessage(String(calcNumDigitalInputs()).c_str());
    gLcd->lcdMessage(" An ");
    gLcd->lcdMessage(String(gPinBlocksAnalog.size()).c_str());
    gLcd->lcdMessage(" ");
    gLcd->lcdMessage(String(calcNumAnalogInputs()).c_str());

    // Lline 4: runtime status, not written here
}

void initAnalogPinsAndFilters() {
    for (auto pbi = gPinBlocksAnalog.begin(); pbi != gPinBlocksAnalog.end(); pbi++) {
        midi::DataByte ccnum = pbi->baseCCNum;
        for (int anPin = pbi->basePin; anPin < pbi->basePin + pbi->numPins; anPin++, ccnum++) {
            // check for patently illegal pin - on GCM4 there are some in the midst of the analog pin range
            // move pin legality checks to pin_list.h / .cpp
            PinList::checkLegalPin(anPin, "Bad AnalogIn Pin");
            // make sure tha analog pin is actually an analog capable pin
            PinList::checkLegalAnalogPin(anPin, "Not AnalogIn");
            pinMode(anPin, INPUT);
            AM_DBG(F("Pin"), anPin, F("AnInput"));

            // init the filter for this pin
            AnalogLPF::addAnalogLPF(anPin, ccnum, pbi->midiOutChan, pbi->lowEndband, pbi->highEndBand, pbi->deadband, pbi->filterAlpha);
        }
    }
}

void setup() 
{

#if USE_DEBUG_PRINT
    // Wait for serial port to connect (needed for USB monitor, happens right away for regular serial)
    Console->begin(consoleBaudRate);
    while( !Console ) {
        ;
    }
    // delay for Grand Central so we can startup serial monitor on the bootloader port
    //delay(10*1000);
#endif
    AM_DBG(gProdName, gProdVersion);
    AM_DBG(gProdCopyright);
    AM_DBG(gProdLicense);

    // need to start LCD before any spindie can happen
    if (gConfig.useLcd) {
        initLCD();
    }

    // read digital and analog pin group definitions from flash or SD card config file
    initMemPinBlocks();

    configurePins();
    initAnalogPinsAndFilters();
    
    initDebouncers();
    
    if (gConfig.useLcd) {
        showStartupBannerOnLcd();
    }

    startMidi();

    // Display as-configured startup banner
}

// local statics for the main loop
unsigned long lastMillis = 0;
unsigned long loopCount = 0;

void loop() 
{
    // scan loop rate tracking
    loopCount++;
    if ((millis() - lastMillis) > 1000) {
        lastMillis = millis();
        int mem = freeMemory();

        // Blnk the standard Arduino LED
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

        AM_DBG(F("rate: "), loopCount);

        if (gConfig.showFreeMem) {
            AM_DBG(F("Mem"), mem);
        }

        // this takes a few msec of blocking foreground time, that's OK if it doesn't increase much
        if (gConfig.useLcd) {
            char buf[13];

            if (gConfig.showFreeMem) {
                // show free memory - max 7 digits (teensy)
                // This is great to verify that memory is not being leaked but costs ~1% scan rate
                gLcd->pLCD->setCursor(0, 3);
                gLcd->lcdMessage("Mem ");
                itoa(mem, buf, 10);
                gLcd->lcdMessage(buf);
            }

            // show scan rate in LR corner of display - allow 5 chars
            gLcd->pLCD->setCursor(12, 3);
            gLcd->lcdMessage("Hz ");
            itoa(loopCount, buf, 10);
            gLcd->lcdMessage(buf);
        }
        loopCount = 0;
    }

    if (gConfig.useEthernet) {
        // Polled read IO: You have to call this periodically for connection requests to be serviced.
        // There is no noticeable overhead - if scanPinBlocksSingleContact() is turned off, MIDI.read() cycles at 11.15 KHz.
        MIDI.read();
    }

    // These substitute scan routines may require that you configure the pins differently in configurePins, not according to the pinBlocks
    // For some you must also reduce the call frequency
    //test_fastread();
    // byte buf[10]; test_diodeMatrix_8x8(buf);

    if (gConfig.logScanSequence) {
        // Divide down scan frequency to one per 10sec to allow for extensive console output
        if (millis() % 10000 == 0) {
            InputScanner::scanDigitalPinBlocks();
            AnalogLPF::filterAllAnalogInputs();
        }
    }
    else {
        InputScanner::scanDigitalPinBlocks();
        AnalogLPF::filterAllAnalogInputs();
    }
}


