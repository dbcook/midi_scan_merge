/*
-------------------------------------------------------------------------------------
MIDI scanner / merger/ decodeer for Arduino and Teensy family processors
-------------------------------------------------------------------------------------

This is a multi-function organ oriented MIDI scanner / merger / decoder for Arduino
architecture processors such as the Arduino Due, Mega 2560, and Teensy. Its primary functions are:

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
a total of 64-65 inputs.  This could allow as many as 7 8x8 diode matrix arrays on the more
powerful processors.
*/

#include <Arduino.h>
#include <Ethernet3.h>
#include <ListLib.h>
#if defined(ARDUINO_AVR_MEGA2560)
#include <MemoryFree.h>
#elif defined(ARDUINO_SAM_DUE)
// MemoryFree library is AVR specific
#endif

// Due: This brings ignorable compile warnings stemming from -Wreorder in Wire.h - members initialized in ctor in different order than declared.
// Apparently only affects SAM processors and not SAMD, we get no warnings on Grand Central.
#include<LiquidCrystal_I2C.h>

#include <MIDI.h>
#include <AppleMIDI.h>

#define GEN_GLOBALS
#include "glob_gen.h"
#include "debug.h"
//#include "nv_mem.h"
#include "stringify.h"
#include "config_features.h"
#include "fastread.h"
#include "debug.h"
#include "data.h"
#include "midi_const.h"
#include "debouncer.h"
#include "pin_list.h"
#include "scanner.h"
#include "lcd_display.h"

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

#if ETHERNET_MIDI_CONNECT
void OnAppleMidiConnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
  gEthConnections++;
  AM_DBG(F("AppleMidi received connect to session"), ssrc, name);
}

void OnAppleMidiDisconnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
  gEthConnections--;
  AM_DBG(F("AppleMidi disconnect"), ssrc);
}
#endif


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

#if ETHERNET_MIDI_CONNECT
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

#endif // ETHERNET_MIDI_CONNECT

} // startMidi


void configurePins() {
    pinMode(LED_BUILTIN, OUTPUT);
#if 0
    // single contact pin blocks
    for (int i = 0; i < nFlashPinBlocks; i++) {
        const PinBlock_t * pb = &gFlashPinBlocks[i];

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
#endif
    //
    // multi-contact digital input pin blocks
    //
    // new code to work from gMemPinBlocksDigital and gMemPinBlocksAnalog
    
    for (auto pbi = gPinBlocksDigital.begin(); pbi != gPinBlocksDigital.end(); pbi++) {
        int nc = pbi->numContacts;
        if (pbi->useSelect) {
            // diode matrix
            for (int j = 0; j < nc; j++) {
                const PbPinInfo_t * pbinf = &(pbi->pbPinInfo[j]);
                for (uint8_t selPin = pbinf->selectBasePin; selPin < pbinf->selectBasePin + pbi->numSelectPins; selPin++) {
                    pinMode(selPin, OUTPUT);
                    digitalWrite(selPin, pbi->activeLow ? HIGH : LOW);
                    AM_DBG(F("Pin"), selPin, F("Output"));
                }
                for (uint8_t readPin = pbinf->readBasePin; readPin < pbinf->readBasePin + pbi->numReadPins; readPin++) {
                    pinMode(readPin, pbi->activeLow ? INPUT_PULLUP : INPUT);
                    AM_DBG(F("Pin"), readPin, F("Pullup"));
                }
            }
        }
        else {
            // parallel digital inputs block
            for (int j = 0; j < nc; j++) {
                const PbPinInfo_t * pbinf = &(pbi->pbPinInfo[j]);
                for (uint8_t readPin = pbinf->readBasePin; readPin < pbinf->readBasePin + pbi->numReadPins; readPin++) {
                    pinMode(readPin, pbi->activeLow ? INPUT_PULLUP : INPUT);
                    AM_DBG(F("Pin"), readPin, F("Pullup"));
                }
            }
        }
    }
    //
    // analog input pin blocks - set as input with pullup disabled
    // The analog pins merely need to not be in output mode - analogRead and digitalRead will both work on an analog pin in input mode.
    // We disable the pullup here in case somehow we enter with the pullup enabled.
    // analogRead will still work if the pullup is on but the readings will be inaccurate
    //
    for (auto pbi = gPinBlocksAnalog.begin(); pbi != gPinBlocksAnalog.end(); pbi++) {
        for (int anPin = pbi->basePin; anPin < pbi->basePin + pbi->numPins; anPin++) {
            pinMode(anPin, INPUT);
            AM_DBG(F("Pin"), anPin, F("AnInput"));
        }
    }
    
}

// Startup for the optional 20x4 LCD display
// This is all harmless if the LCD is not connected as long as nothing else is on I2C address 0x27
// IMPORTANT: Any LCD messages to be displayed during normal MIDI scanning operation need to be done in a background thread
//  due to significant hard waits.  Crash messages can and should be done in the foreground.
// TODO make into an LCD handling class since we'll want a ringbuffer deque of message objects queued for the background thread
//

// This just instantiates a global object.
// TODO make this a member in our class so we don't consume the memory if not configured.
// TODO The I2C takes 2 pins - in Mega format large-IO boards these are digital 20-21.
LcdDisplay * gLcd = new LcdDisplay();
void initLCD() {
    gLcd->init();
    // Display initial startup banner
    gLcd->lcdMessage("DBCook MIDI ", 0, 0);
    gLcd->lcdMessage(gProdVersion);
}

void showStartupBannerOnLcd() {
    // leave row 0 alone for init msg with version

    // Line 1: configuration - enabled features etc
    gLcd->pLCD->setCursor(0, 1);
    if (gUseEthernetMidi) {
        gLcd->lcdMessage("ETH ");
    }
    if (gUseUSBMidi) {
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

void setup() 
{

#if USE_DEBUG_PRINT
    // Wait for serial port to connect (needed for USB monitor, happens right away for regular serial)
    Console->begin(consoleBaudRate);
    while( !Console ) {
        ;
    }
    AM_DBG(gProdName, gProdVersion);
    AM_DBG(gProdCopyright);
    AM_DBG(gProdLicense);
#endif

    initLCD();

    initMemPinBlocks();
    configurePins();
    

    // *** fix these to work from gMemPinBlocks
    initDebouncers();
    initDebouncerBases();

    showStartupBannerOnLcd();

    startMidi();

    // Display as-configured startup banner
}

// local statics for the main loop
unsigned long lastMillis = 0;
unsigned long loopCount = 0;

void loop() 
{
    // loop rate tracking data for Arduino Mega 2560
    //   For reasons yet to be determined, diode matrix scanning time is a little nonlinear vs matrix size.
    //   It seems like the higher number IO pins may be slower - branch offset efficiency vs offset byte size?
    //    two 8x8 matrix blocks (Mega, slow IO)                  0.641 KHz (1560 usec = 12.2 usec per input )
    //    single block of 32 parallels (Mega, slow IO            3.48 KHz (287 usec = 8.97 usec per input)
    //    four 8x8 blocks (Due, fast IO)                         0.88 KHz (1132 usec = 4.64 usec per input)
    loopCount++;
    if ((millis() - lastMillis) > 1000) {
        lastMillis = millis();
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        AM_DBG(F("rate: "), loopCount);
#if defined(ARDUINO_AVR_MEGA2560)
        Console_print(F("memfree: ")); Console_println(freeMemory());
#elif defined(ARDUINO_SAM_DUE)
#endif
        loopCount = 0;
    }

#if ETHERNET_MIDI_CONNECT
    // Polled read IO: You have to call this periodically for connection requests to be serviced.
    // There is no noticeable overhead - if scanPinBlocksSingleContact() is turned off, MIDI.read() cycles at 11.15 KHz.
    MIDI.read();
#endif

    // These substitute scan routines may require that you configure the pins differently in configurePins, not according to the pinBlocks
    // For some you must also reduce the call frequency
    //test_fastread();
    // byte buf[10]; test_diodeMatrix_8x8(buf);

#if LOG_SCAN_SEQUENCE
    // Divide down scan frequency to one per 10sec to allow for extensive console output
    if (millis() % 10000 == 0) {
        InputScanner::scanPinBlocksSingleContact();
    }
#else
    InputScanner::scanPinBlocksSingleContact();
#endif
}


