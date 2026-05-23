# Flexible MIDI Scanner - Merger - Decoder for Arduino and Teensy

This PlatformIO based project targets Arduino family processors with large digital IO pin counts,
and is aimed at exploiting the number of IO pins in order to scan up to four 8x8 diode matrix groups
representing up to 256 discrete contacts, and possibly more with a fast processor (Arduino Due,
AdaFruit Grand Central M4, or Teensy) and shared scan pins.

We perform high precision switch debouncing and emit MIDI NoteOn and NoteOff messages accordingly
over various transports (serial, Ethernet, USB).

The system context for this is to support musical instruments such as pipe organs - actual or virtual - that
may have a large number of inputs:  four or more 61-note keyboards, a 32-note pedalboard, up to 100 momentary-contact
thumb buttons ("pistons" in traditional organ lingo), up to 25-30 toe pistons, and 3-6 analog expression pedals ("shoes").
These instruments can approach 500 total inputs with single-contact keyboards and 800 with dual-contact
velocity sensing keyboards.  Theater organs with triple contact aftertouch can exceed 1000 inputs.
Thus in all cases multiple scanner/encoders will be needed, so maximizing their capacity is useful
to control cost and system interconnects.

The development is emphasizing the Ethernet MIDI transport, which offers low latency, high throughput,
and much better network infrastructure than either the old 31 Kbps serial MIDI or USB-MIDI.
Apple Macs all have built-in Ethernet MIDI support, making system setup a snap.  USB is quite fast
enough but the physical setup requires hubs that can be flaky, and node-to-node distances beyond
5 meters are problematic.  By contrast, 100Mbps Ethernet cables can easily run 100+ meters, and
widely available $50 network switches provide many ports of fully matrixed connectivity.

## Features Summary

* High performance input scanning of contacts and generation of MIDI note on/off messages
* Supports diode matrix and parallel input blocks in any combination
* Time-based debounce of both attack and release
* Runs on fast or slow CPUs without changes
* Robust serial MIDI thru/merge handling of all message types
    * Optional channel separation remapping of channel messages
    * Number of merge input channels configurable up to 3 (all of the hardware serials on the Due and Mega 2560)
* Parameterized to accommodate various input hardware without extensive code changes
* Performance stats written to debug console
* Built with PlatformIO plugin for vscode

## Future Features

* Analog input support (pedals, rotary encoders, sliders) with deadband and lowpass filtering is in work
* Board support for AdaFruit Grand Central M4:
* Board support for Teeensy 4.1
* Programmable parameters via SD card, writeable from any computer
* Multi-contact keyboard scanning
    * Velocity sensing (dual contact)
    * Aftertouch (triple contact)
* MIDI decoder - consume messages on one or more channels and activate outputs
    * LEDs
    * Multi-digit displays (e.g. sequence frame number)
    * Solenoid drivers (e.g. for electromechanical organ stops)
* Support IO extenders (shift registers etc.)
* Adjunct Hardware
    * Diode matrix boards for parallel keyboards
    * General purpose carrier and interconnect board for Teensy 4.1
    * LED driver (matrix based) for MIDI decoder

## Supported Boards

[__AdaFruit Grand Central M4 Express__](https://www.adafruit.com/product/4064)

[__Arduino Due__](https://store-usa.arduino.cc/products/arduino-due)

[__PJRC/SparkFun Teensy 4.1__](https://www.sparkfun.com/teensy-4-1.html) - future

## Things You Need to Know


### Scan Cycle Rate

How fast does it really need to be?  Scan latency with a scan cycle time of 2 milliseconds (rate of 500Hz) is 1.0 +/- 1.0 milliseconds, i.e.
half the scan cycle time, with a minimum of 0 and a maximum of 2.0 msec.  Compared to the needed debounce
time of 15-20 milliseconds, 1 msec is only 5-8% of the total latency.  Thus you can have as low as 200 Hz scan rate
before the scan latency reaches 20% of the total.

### Ethernet MIDI Transport

* For AppleMIDI Ethernet to work, your computer and the scanner devices __must__ be on the same network segment.
This is required by the Bonjour discovery protocol which uses nonroutable broadcast addresses.  The practical
meaning is that your computer needs a wired Ethernet connection and must be connected to the same LAN as
your MIDI processors.

* __Mac Hosts:__ All Mac computers can connect directly to a device running this firmware with zero added software.  Support
is very mature and built-in via the Apple MIDI Studio module that resides in Settings | Audio MIDI Setup.

* __Windows Hosts:__ Microsoft has just (early 2026) released an 
[in-box UMP MIDI driver together with downloadable apps](https://github.com/microsoft/MIDI/releases) following
years of nothing but 3rd party MIDI support.  Reports on how-to-use and whether it works would be
appreciated as I have no suitable Windows systems to test it on.  The driver is now baked in to recent
Windows versions but the MIDI apps have to be downloaded separately.  The good news is that apps are
all open source. Tobias Erichsen's [rtpMIDI](https://www.tobias-erichsen.de/software/rtpmidi.html) has
been the standard 3rd party solution for quite some time, though Microsoft's major updates have reportedly
made occasional restarts needed of late.

### USB Hubs and Cables

When using USB-MIDI transport to the computer, USB hubs are often needed.  I have seen many reports of USB
connectivity problems that are often - wrongly or rightly - laid to poor USB hubs.  As a long time engineer
I would say that cables are almost as likely as hubs to cause trouble, and when making a USB system of
any complexity it's vital to use good hubs and cables alike.  My personal preference (and this is a non compensated
opinion!) is for Anker hubs and cables.  I don't think I've ever had an Anker hub die, and their braided
jacket cables - while not 100% indestructible - last much longer than generic ones.

You want braided cables.  The outer braid makes the cable stiffer and provides a lot more protection against
over-flexing the conductors, a major cause of cable failure.  Cheap thin ultra flexible cables are much
less robust.

The other thing to look at for cables is the transition between the main cable and the plug housings.
An abrupt change in diameter from the cable to the housing is bad because it creates a stress concentration
point that will lead to early failure.  A gradual tapered transition of 10-15mm is much better.

## State of the Project

The fourth and current prototype adds working Ethernet MIDI transport and replaces the Mega 2560
build with an operational Arduino Due build.

Since the 3rd prototype, the Ethernet libraries have been integrated up to the point where
session connection and note emitting code is in place. The Ethernet transport now
succesfully handles session establishment and disconnection from a Mac via the built-in
Audio MIDI Setup, and note on/off commands flow end-to-end.
Current work is focused on adding analog input processing and low-pass filtering.

I have a Grand Central M4 on order and already have a couple of Teensy 4.1
units, though not with Ethernet jacks nor breakout boards.  I also just got a cheap and
easy-to-disassemble Nektar GX-61 MIDI controller which will be used for various test purposes
and debug of two-contact velocity sensing.

Grand Central M4 and Arduino Due use a different MCU architecture (SAM vs AVR) so there will be separate configs for them
in `platformio.ini`.  Right now the Due config runs and a Grand Central config will be added shortly.

### Current Hotspots

#### Parameter and Config Loading

So far, configuration and parameters are all set up in flash code.  That is not a production
solution, where we want one code build to read in its configuration and operate accordingly.

I'm probably going to choose micro SD cards with configuration via a .yml file.
The Grand Central and Teensy have SD card slots on board, and the Due can have one if an Ethernet shield is present.
All have plenty of RAM and flash for a YAML parser, though the existing Arduino YAML parser
isn't compatible with the Due's early gen SAM architecture.

The pros for this approach are that SD
cards are very standard and can be externally programmed on your computer.  The cons are
a small extra cost, and a non-small extra cost if you have to put an Ethernet shield on a Due
that is only going to be used with USB transport.  The latter problem is easily cured by
just using a Grand Central or Teensy instead, so I'm still leaning toward SD cards.

I was initially looking at using onboard or emulated EEPROM interfaces, but after realizing that
config storage has to be programmable externally for our UI-less application I've dropped that idea.
The libraries and APIs are also very disparate between boards and it would take a lot of work to impose a common wrapper.
The MIDI processors are also not going to have UI hardware, so getting a fairly extensive config into the unit
would involve a lot of code.

#### Serial Console Output

It's very useful - almost mandatory - to be able to see the serial console output when setting up
a new configruation.  When you are not using USB MIDI transport, this can be done easily by plugging
in a USB cable to your computer and using any terminal program to monitor it.

However, when USB-MIDI is in use, you have to put the serial monitor on a different port.
Unless you have multiple USB ports, you will need to hook up a USB-serial adapter to the
serial monitor port.  That may a bit much for most users so I am going to try to make
it easy by providing various tested config files for USB-MIDI transport that can be used and
sligthly modified with high confidence.

On the Due with its dual USB ports, the console output comes out on the programming port.
For MIDI USB transport, you just need to put your MIDI library object on the native port `SerialUSB`.

#### Memory Diagnostics

The `MemoryFree` library originally used to measure memory consumption on the Mega only works on the old AVR architecture.
On the Due SAM processor the heap boundary symbols are different.  For now this is of no consequence since it doesn't
look like memory will be an issue on the Due. There is [code suitable for SAM here]()


## Contributing

*Bug reports* Please use GitHub issues in the normal way.

*Fixes and enhancements* Plase follow the conventional open source process:  fork this repo,
create a branch with your changes on it, and submit a pull request (PR) for that branch. Be sure
to generally follow the conventions used in this project; nonconforming PRs will not be accepted.

## Prerequisites and Building

To configure, build and load this software into an Arduino, you need vscode with at least these plugins:

* Microsoft C/C++
* PlatformIO

On Apple Silicon Macs, you need to install rosetta 2 to allow emulation of the x86 compiler used
for the older AVR based Arduinos (Mega/Uno/Nano).

```
softwareupdate --install-rosetta
```

Due to the need to save RAM and keep as much data in flash as possible, you have to do a little
configuration in the code to specify your diode matrix and parallel input configuration. Support
is provided to specify a separate MIDI output channel and MIDI note base for each diode matrix group.


## Console Output

The firmware will spit out useful periodic messages on the primary serial port.  This port is shared
with the bootloader, so if you add a MIDI serial shield you should modify it to use a different hardware
serial port in order to allow these messages to be seen and avoid having to flip the prog/run switch
every time you load code.

Console output sent to `Serial` on the Due comes out the Programming port.

## Detailed Documentation

[Ethernet Shield 2 with Arduino Due/ Mega 2560 Pin Availability](docs/ethernet-shield.md)

[Generic MIDI serial shield pins and modification](docs/midi-shields.md)

## MIDI Transport

This package will support multiple transport methods - serial, USB and Ethernet - built around the family of MIDI libraries
that have been developed around the 47Effects core library.  As a practical matter, the original 31kbps serial MIDI using
the DIN-5 connectors is too slow for an organ, especially if daisy-chained.  Therefore the USB and Ethernet
RTP-MIDI transport methods are much preferred, though serial MIDI is being used for testing.

If USB transport is wanted, you have to use a more recent Arduino board like the Due or Leonardo that has full class-compliant
USB host capability.

For Ethernet capability, you can either use a Teensy 4.1 with its onboard Ethernet, or an Arduino Due with an Ethernet shield.
Of these, the Teensy is much less expensive and vastly more powerful, so it wins by a mile if you can make a suitable
carrier board for it. Both can be powered by USB.

## Operation

### Debouncing

Switch debounce is performed for both the "make" and "break" actions.  20 millisec is usually considered sufficient
for the "make".  The break action is separately configured so it can be shorter.  5 milliseconds seems to be
be a reasonable value that allows a very fast but reliable note repeat.

### Performance

The scan cycle rate is accurately measured using the Arduino millisecond timer and emitted on the serial
console port at 1 Hz, allowing the effects of algorithm changes to be seen immediately.

### MIDI Merge and Daisy Chaining

If a serial MIDI shield is used, the software can be configured to perform a merge operation, where messages
arriving on the serial MIDI link are sent onward over the output transport link.  MIDI channel separation remapping
is implemented if that is needed for merging inputs from multiple scanners that are emitting notes on the same
channel.  

It is possible for a few of these Arduino scanners to be daisy-chained by configuring for both serial
output and input, or serial input with Ethernet output.
Multiple serial inputs are supported in the software, though the inexpensive
MIDI serial shields cannot be stacked without some hardware hacking.
But daisy chaining is not a good idea from a latency perspective;
chaining of serial MIDI ports introduces 2+ msec MINIMUM of delay per hop, so single notes
from the most distant manual in a 4-manual daisy chain could be delayed by as much as 10 msec.
The last note in a big chord from the end of the chain could see 20-25 msec of latency.

### MCU Board Details

This section explains in detail the boards that are suitable to use with this firmware. The application with its
full feature set is fairly demanding, and I am targeting the ability to handle all the inputs for a good
sized organ (up to 448 inputs) with one processor.

#### System Requirements

The minimum system requirements are:
  * 32 KB SRAM
  * 128 KB flash
  * 100 MHz clock
  * Class compliant USB host port onboard
  * Available micro SD card slot (onboard perferred)
  * Available Ethernet
  * Board support exists in PlatformIO

#### The boards

* [__AdaFruit Grand Central M4 Express__](https://www.adafruit.com/product/4064) - $40.
This board uses the well known Arduino Mega/Due pin layout.
It has an ATSAMD51 CPU  (gen M4) with 120 MHz clock, 256 KB of SRAM, 1 MB of flash, full USB host capability,
and a micro SD card slot.
There are 70 total IO pins of which 16 can be analog inputs.
It also has an 8MB onboard QSPI flash data store that is externally writable through the USB port and could be
used for configuration files.  It will accept the standard Arduino Ethernet Shield 2 ($30).
   * USB-MIDI transport: works standalone, no extra hardware
   * Ethernet MIDI transport: requires Ethernet Shield 2
   * Serial MIDI transprot: requires 3.3V capable MIDI shield attached to a hardware serial port

* [__Arduino Due__](https://store-usa.arduino.cc/products/arduino-due)
This official Arduino board is over 10 years old but still useful.  It features an 84 MHz SAMD M3 chip, 96KB of
SRAM (two banks, 64+32), 512 KB flash, and full USB host capability.  On this board there are 70 total IO pins
in the same layout as the Grand Central, but only 12 are analog capable inputs.  It does not have an onboard
SD card slot but can utilize the on on the Ethernet shield at the price of 3-4 IO pins.
It costs $50, making it noticeably more
expensive than the Grand Central despite having an older generation CPU and a lot less memory.  It also accepts
the Arduino Ethernet Shield 2.
   * USB-MIDI transport: requires Ethernet Shield 2 to host SD card
   * Ethernet MIDI transport: requires Ethernet Shield 2
   * Serial MIDI transport:
      * Requires Ethernet Shield 2 to host SD card
      * Requires 3.3V capable MIDI shield attached to a hardware serial port

* [__PJRC/SparkFun Teensy 4.1__](https://www.sparkfun.com/teensy-4-1.html)
The Teensy 4.1 is in many ways the king of low cost MCUs.  It has a screaming 600 MHz two instruction-per-clock
Cortex M7 chip with 64-bit hardware FPU, 512 KB of single-clock RAM (1 MB total RAM), 8 MB of code flash (256KB reserved),
18 analog capable inputs out of 55 total IO pins, onboard Ethernet PHY, and a micro SD card slot.  The core module
only costs $31.50, but you will need an Ethernet jack costing a few dollars, and likely a screw terminal
breakout board, which can be $15 to $25.  It does have 15 fewer total IO pins vs the Grand Central and Due,
which limits the scope of organ that can be handled with one processor.
   * USB-MIDI transport: works standalone
   * Ethernet MIDI transport: requires Ethernet jack and ribbon cable ($5-10)
   * Serial MIDI transport: requires 3.3V capable MIDI shield attached to a hardware serial port

#### Board Feature Commonality and Differences

* The AdaFruit Grand Central M4 and Arduino Due have the same number of IO pins (70)
on the same physical board layout.  Both will accept the Arduino Ethernet Shield 2, which has an
SD card slot.

* The Grand Central has an onboard SD card slot, meaning that if we adopt that as the standard means
of configuration, the Grand Central is totally self sufficient for USB MIDI transport.
The Arduino Due needs an Ethernet Shield or something similar to provide an SD card slot
for configuration, even if Ethernet MIDI transport is not being used.

* Both the Grand Central and Due will easily support the maximum number of digital IO inputs that is achievable
via diode matrix on 64 usable pins (448).

This table summarizes the capabilities and cost of the known suitable boards.
Both the Grand Central and Arduino Due need an Ethernet Shield ($30, included in the Cost column) if
Ethernet MIDI transport is needed. The Due needs the Ethernet shield to house the SD card slot regardless of the MIDI transport.
This makes the Due much more expensive than the Grand Central for USB-MIDI transport.

The Teensy 4.1 costs $31.50 with onboard Ethernet PHY and an SD card slot.  It only needs an ethernet
connector with magnetics and a carrier board (est. $15), making it the least costly option
though it has 15 fewer IO pins than the Grand Central and Due.

| Processor              | RAM       | Flash     | Speed     | Dig Pins  | Usable | USBHost | SD Card   | Cost | Remarks
| ----                   |----       |----       |----       |----       |----    |----     |----       |----  |----
| AdaFruit Grand Central | 256 KB    | 1 MB      | 120 MHz   | 70        | 65     | YES     | YES       | 70   | Fast, plenty of RAM and pins
| Arduino Due            | 96 KB     | 512 KB    | 84 MHz    | 70        | 66     | YES     | EthShield | 80   | Fast enough, eth $30
| Teensy 4.1             | 1 MB      | 7936 KB   | 600 MHz   | 55        | 50?    | YES     | YES       | ~50  | Very fast, Eth PHY onboard

#### Processors Not Considered Suitable and Why

The following are examples of processor modules that are not suitable for use with this firmware.  The principal reason is not
having enough RAM to accommodate the full slate of MIDI transport libraries, the config parser, and a reasonable number of
input debounce/filters.

The Arduino Mega 2560 was once the standard for high pin count Arduinos, but it is now very old and has been 
completely obsoleted by the Arduino Due and AdaFruit Grand Central.
It is possible - and I did this during the prototyping phase of this project - to make a firmware build that will process
3-4 keyboards on the Mega 2560, but it has to be completely configured in code, which is not a production-worthy solution.
Adding the Ethernet library took the capacity down to
two keyboards, and adding the SD card loadable config support looked like it would further reduce that.
The Mega 2560 is also fundamentally incapable of doing USB-MIDI transport due to the lack of class-compliant USB
host support.

The Arduino Leonardo has *extremely* small RAM and a pin count that is just enough to allow scanning a single matrix
keyboard (61 inputs) and emitting the MIDI over USB.  It would need to have a fixed configuration with a separate build for
each output MIDI channel.  The Leonardo would be a very expensive solution for a full organ which can have 400-600 inputs that
would require 7-10 Leonardos.  Due to library RAM/stack needs I think that running the AppleMIDI Ethernet transport
onto the Leonardo is most likely infeasible as well.

| Processor     | RAM       | Flash     | Speed     | Dig Pins  | Usable | USBHost | SD Card   | Cost     | Remarks
| ----          |----       |----       |----       |----       |----    |----     |----       |----      |----
| Ard Mega 2560 | 8 KB      | 256 KB    | 16 MHz    | 70        | 66     | NO      | EthShield | 50 + 30  | Not enough RAM, not fast enough for a full organ
| Ard Leonardo  | 2.5 KB    | 32 KB     | 16 MHz    | 20        | 18     | YES     | infeasible | 24.     | Insufficient RAM, low pin count


### System Voltage 3.3V Considerations

All of the suitable MCU boards are 3.3V systems and cannot take 5V or higher input signals directly.  If you are using the
conventional active low open-drain inputs via the internal pullups (typical for simple contact closures), you can use
the 3.3V systems with no extra hardware.  If you need conversion, the four-channel 
[Noyito Optocouplers](https://www.amazon.com/NOYITO-4-Channel-Optocoupler-Photoelectric-Converter/dp/B07TDYW5FF?th=1)
and similar producst may be of interest.  For full keyboards you will probably want a higher-density solution.


## Constraints

Overall on the recommended processors there is enough memory and enough CPU to scan and debounce 300-448 inputs in up to seven 8x8
diode matrix groups at a rate of at least 500 Hz.

If parallel inputs are used, you can only scan 64 total inputs, so memory and speed will
not be a problem on any Arduino or Teensy MCU.  In this case you are completely pin limited
and an IO expander or shift register board could be very helpful.

### Memory

Nearly all RAM is statically allocated to avoid problems with unpredictable heap usage.
The build output in the terminal will tell you how much of the static RAM and flash are used.

The debouncers as currently implemented use 16 bytes of RAM each.

### CPU

The Arduino Due can scan 7 8x8 keyboard matrix blocks (448 inputs total) at 540 Hz.

### IO Capacity

Both the Grand Central and Due have 70 total IO pins, all of which can be programmed as digital inputs or outputs.
Out of the set of 70 pins, various subsets can be programmed for other functions such as 
analog input, serial ports, PWM output, and some specialty functions.  Most IO shields
will consume a few pins.  On a Grand Central or Due with an Ethernet Shield 2 installed, the
following 5 pins are not usable:

* Pins 0 and 1:  the base serial port used by the bootloader
* Pins 4 and 10: used as chip selects by the Ethernet shield
* Pin 13: the standard Arduino LED output

Thus With the Ethernet shield attached, on Grand Central and Due you have exactly 65 usable IO pins,
of which 16 (Grand Central) or 12 (Due) are analog-capable.
This will physically handle 4 full 8x8 diode matrix keyboards without using shared lines,
or up to seven 8x8's if the same scan or read lines are used for each matrix.  Using two or more of
the analog-capable pins for analog inputs OR using more than one digital pin for parallel inputs will
reduce by one the number of 8x8 diode matrix groups that can be supported.

If a MIDI serial shield is used and you do not want to have to flip PROG/RUN switches everytime you load code,
you have to give up two more IO pins, leaving only 63 usable if an Ethernet shield is also present.
This is a good reason not to have a serial MIDI shield attached unless it is actually used.
See [docs/midi-shields.md](docs/midi-shields.md) for specifics on wiring serial MIDI shields.

The largest number of 8x8 diode matrix groups that can be scanned with 64 pins is seven,
provided that common scan or read outputs are used for all of the matrix groups.  Each independent
set of scan/read lines consumes 8 pins.  Thus if you use independent scan and lines for each matrix,
you can only do 4 8x8 groups.

IO expanders can possibly be used, though most of them are I2C based which has
speed limitations.  MCP23017 based expanders can have the wire rate set as high as 1.7 MHz,
but even that is too slow to support high IO bit counts.  SPI based expanders can clock
at 10MHz, which is considerably better.


### IO Efficiency

Initial testing with Arduino Mega 2560 showed that using the Arduino standard pin IO
routines digitalRead() and digitalWrite() had a significant adverse impact on scan loop
speeds, taking about 40% of the CPU per scan iteration.

The reason is that these calls include various safety checks and mapping, and cost ~100 cycles
per input line (said to be ~3 usec on a 16 MHz Arduino whereas a direct op is ~125 nanosec).

It turned out to be drastically faster to use the digitalWriteFast library (which also includes
read functions).

In order for digitalReadFast() to work the pin number has to be a compile time constant.  Thus to use this with our
flexible pin block definition scheme we need a function that resolves a pin number in a variable to
the specific digitalReadFast macro for that pin.  That has been accomplished using an array of
function pointers stored and a simple wrapper function that invokes the correct macro.
This adds a single-digit number of clocks per invocation but is still about 10x faster than
using digitalRead().

An implementation using whole-byte reads of the CPU ports was considered but not adopted, because
the mapping of IO pins to the CPU ports is irregular and it doesn't look like the additional
reduction in clocks per IO read would be enough to matter.

A theoretical further speedup would be to write code for predefined layouts of common pin blocks (8x8 and 
4x8 diode arrays, plus 32 pin parallel) using fixed pin assignments for the select and read lines.
This could explicitly use digitalReadFast and digitialWriteFast with no additional overhead, but 
would be tedious to maintain because the loops would have to be completely unrolled.

### Velocity and Aftertouch Sensing

Velocity sensing for keyboards (not yet implemented) will double the number of contacts and will
double or nearly double the memory consumption of each debouncer.  It will also slow down the scanning slightly
due to increased complexity of the algorithm while a key has at least one contact closed.
However I think it should be possible to do velocity sensing on a 61-note keyboard with a single
Mega 2560 at around 1 KHz.

Aftertouch sensing, used on theater organs and synthesizers, involves 3 contacts per key.
Implementing it just means stacking a 3rd layer of debounce atop the velocity sensing logic.
Because of the number of IO pins needed, it will consume 3 8x8 matrix slots.  Thus without
sharing scan lines or an IO extender you will only be able to scan one such keyboard per Arduino,
and it will need to be a Due because three 8x8 blocks is too much for a Mega 2560.

# A Short History of the Development

Originally a prototype was implemented using serial MIDI transport that seemed to run quickly
on the Mega 2560.  However the straightforward code design ended up using 35-40% too much memory,
even with limited features and no user configurability.

A second version was built that reduced RAM usage by enough to get
the data for a full slate of four 8x8 diode matrix arrays into about 4KB of RAM.  The time per
input scanned was about 9-12 usec depending on circumstances.
Several matrix and block scenarios appeared to run correctly. The debugging of this version 
used a generic serial MIDI shield with the hardware hacked to attach it to the Mega 2560 SER3 port.

After discovery that the stock Arduino digital IO library routines were eating up an
excessive amount of CPU, a third version was implemented that uses the digitalWriteFast
libary to reduce the IO overhead and give a scan time per input of about 7-8 usec
on the Mega 2560.


## MIDI DIN-5 Serial Cables

This reference information is in case you have to work with old school serial MIDI.

__MIDI DIN-5 connector pinout__

|PIN    |  MIDI IN                 | MIDI OUT               | MIDI THRU
----    |-----                     |------                  |-----
1       | NC or GND                | NC or GND              | NC or GND
2       | SHIELD                   | SHIELD                 | SHIELD
3       | NC or +V                 | NC or +V               | NC or +V
4       | MIDI source IN           | MIDI source (pullup)   | MIDI source (pullup)
5       | MIDI sink (opto input)   | MIDI sink (driven)     | MIDI sink (driven)

The actively driven line (driven by MIDI OUT) is always the MIDI current sink (Pin 5).
This line is driven active low by the MIDI OUT side.
The MIDI current source is just a +5V supply through a 220 Ohm pullup resistor.
On the MIDI IN connector, the data signal on pins 4/5 is typically hooked to an optoisolator
so that when the MIDI OUT side pulls the sink low, current flows through the LED of the
optoisolator on the MIDI IN side.

The MIDI recommended circuit connections are shown here:
https://learn.sparkfun.com/tutorials/midi-tutorial/hardware--electronic-implementation

Pin 2 is always the shield of a shielded 2-conductor cable.

DIN5 MIDI cables should be wired straight thru.  Standard cables only have 2 conductors + shield (pins 2, 4, and 5).

### Crumar MOJO 61B Lower Manual Special Powered MIDI Connection

The Crumar lower manual for the Mojo 61 is of some interest because it is one of the thinnest and
most stackable commercial MIDI keyboards in current production.  It has no physical controls whatsoever,
making it 1/2 to 3/4" thinner than most other MIDI controllers.  This item has been in production
since 2017.

It is a bit unusual because it only needs a DIN-5 MIDI connection between itself and the "full" Mojo 61 that
sits atop it.  Power is provided via this cable, which is possible because in the usual MIDI connector pins 1 and 3
are unused.  A cable for this unit must have 4 conductors plus the shield, with +V on pin 3 and power GND on pin 1.

Crumar states [in this KB article](https://lnx.crumar.it/help/knowledgebase.php?article=3) that the lower keyboard
does not use standard MIDI protocols (not specified whether power or messages) and that in order to use
the lower manual as a standalone MIDI controller you need a device called an MJU "MIDI Jack USB" which
can be seen [here](https://www.gmlab.it/#projects).  This device provides the power-over-MIDI and also is
stated to be configurable as a MIDI event processor.  It's [open source](https://github.com/ZioGuido/GMLAB_MJU)
and built versions are available from [My Rig Shop](http://www.myrigshop.com/)
