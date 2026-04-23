# Flexible MIDI Scanner - Merger for Arduinos

This PlatformIO based project targets the Arduino Mega 2560 and is aimed at exploiting its large number
of IO pins in order to scan up to four 8x8 diode matrix groups representing up to 256 discrete contacts.
It performs high precision switch debouncing and emits MIDI NoteOn and NoteOff messages accordingly.

The system context for this is to support musical instruments such as pipe organs - actual or virtual - that
may have a large number of inputs:  up to 5 61-note keyboards, a 32-note pedalboard, up to 100 momentary-contact
thumb buttons ("pistons" in traditional organ lingo), up to 25-30 toe pistons, and 5 analog expression pedals ("shoes").
This number can approach 500 total inputs with single-contact keyboards and 800 with dual-contact
velocity sensing keyboards.  Thus in all cases multiple scanner/encoders will be needed, so maximizing
their capacity is hepful.

## State of the Project

A prototype scanner was implemented that seemed to run quite quickly.  However the
straightforward code design ended up using 35-40% too much memory and would only have supported two
8x8 and one 8x4 diode matrix groups.

A second version has been built that reduces RAM usage by enough to get
the data for a full slate of four 8x8 diode matrix arrays into about 4KB of RAM.  The time per
input scanned is about 9-12 usec depending on circumstances.  This scanner has passed several
logging tests to prove that the pin block definitions are being read and scanned properly.
Several matrix and block scenarios appear to run correctly.  Testing with actual input
switches is about to begin.  Thus far all of the debugging has used the generic serial MIDI
shield with the hardware hacked to attach it to the Mega 2560 SER3 port.

The Ethernet Shield 2 is installed on the test rig but library integration has not begun.

## Features Summary

* High performance input scanning of contacts and generation of MIDI note on/off messages
* Time-based debounce of both attack and release
* Runs on fast or slow CPU without changes
* Robust MIDI merge handling of all message types including SYSEX
    * Optional channel remapping of channel messages
    * Number of merge input channels configurable up to 3 (all hardware serials on the Mega 2560)
* Parameterized to accommodate various input hardware without extensive code changes
* Performance stats written to debug console
* Built with PlatformIO plugin for vscode


## Prerequisites and Building

To configure, build and load this software into an Arduino, you need vscode with the PlatformIO plugin.

Due to the need to save RAM and keep as much data in flash as possible, you will need to do some
configuration in the code to specify your diode matrix and parallel input configuration. Support
is provided to specify a separate MIDI output channel and MIDI note base for each diode matrix group.

There is enough
memory and CPU to scan and debounce 256 inputs (four 8x8 diode matrix groups).  Nearly all the RAM is
statically allocated to avoid problems with unpredictable heap usage.  The Platform IO build output
in the terminal will tell you how much of the RAM and flash are used.

The firmware will spit out useful periodic messages on the primary serial port.  This port is shared
with the bootloader, so if you add a MIDI serial shield you should modify it to use a different hardware
serial port in order to allow these messages to be seen and avoid having to flip the prog/run switch
every time you load code.

## Detailed Documentation

[Ethernet Shield 2 with Arduino Mega 2560 Pin Availability](docs/ethernet-shield.md)

[Generic MIDI serial shield pins and modification](docs/midi-shields.md)

## MIDI Transport

This package supports multiple transport methods - serial, USB and Ethernet - built around the family of MIDI libraries
that have been developed around the 47Effects core library.  As a practical matter, the original 31kbps serial MIDI using
the DIN-5 connectors is too slow for an organ, especially if daisy-chained.  Therefore the USB and Ethernet
RTP-MIDI transport methods are preferred.

If USB transport is wanted, you have to use a more recent Arduino board like the Leonardo that has full class-compliant
USB host capability.  However, these have far fewer input pins; on a Leonardo you can only support one 8x8 matrix,
and it only has 2.5KB of RAM, about half of which will be consumed by 64 debouncers.

The result is that for maximum IO capacity and lowest MIDI latency, using the Mega 2560 with an Ether Shield 2
is apparently the most powerful option.


## Operation

### Debouncing

Switch debounce is performed for both the "make" and "break" actions.  20 millisec is usually considered sufficient.

### Performance

The initial version of this package runs the scan loop for a single 8x8 matrix (64 inputs, 1 typical keyboard)
at about 10 kHz.  Scan rate is accurately measured using the Arduino millisecond timer and emitted on the serial
console port at 1 Hz, allowing the effects of algorithm changes to be seen immediately.  The processing is very
linear so that the expected scan rate for four 8x8 matrices (256 inputs) should be better than 2 KHz.

### MIDI Merge and Daisy Chaining

If a serial MIDI shield is used, the software can be configured to perform a merge operation, where messages
arriving on the serial MIDI link are sent onward over the output transport link.  MIDI channel remapping is possible
though not enabled by default.  Multiple serial inputs are supported in the software, though the inexpensive
MIDI serial shields cannot be stacked without some hardware hacking.

It would be possible for a few of these Arduino scanners to be daisy-chained by configuring for both serial
output and input.  But this is not really a good idea from a latency perspective;
daisy chaining of serial MIDI ports introduces 2+ msec MINIMUM of delay per hop, so single notes
from the most distant manual in a 4-manual daisy chain could be delayed by as much as 10-12 msec.
The last note in a big chord from the end of the chain could see 20-25 msec of latency.


## Constraints

### Memory

The Arduino Mega 256 has only 8KB of RAM, representing a serious memory challenge.  The debouncers as
currently implemented use 14 bytes of memory each, so 256 of them use 3.6K of RAM.  With the Ethernet
and MIDI libraries there is very little
left over - even keeping a list of pointers to the debouncers costs 1KB, so the configuration code where
you define the diode matrix and parallel inputs has to be entirely flash-based and use computed indexes to
the debouncer objects.

### CPU

The 16MHz Mega 2560 seems to have enough grunt to handle its full capacity inputs with a scan rate near 2 Hz,
which is entirely sufficient.

### IO Pins

Using the Ethernet Shield 2, the Mega 2560 has enough free IO pins to handle 4 full 8x8 diode matrix keyboards,
or 3 8x8 matrices plus 16 analog inputs (expression pedals, rotary encoders, sliders/drawbars).

If a MIDI serial shield is used and you do not want to have to flip PROG/RUN switches everytime you load code,
you have to give up two IO pins and you cannot support four 8x8's.

### Velocity Sensing

Velocity sensing for keyboards (not yet implemented) will double the number of contacts and is expected to
double or nearly double the memory consumption of each debouncer.  It will also slow down the scanning somewhat
due to increased complexity of the algorithm.  However I think it should be possible to do velocity sensing
on two 61-note keyboards with a single Mega 2560 at better than 1 KHz.

## MIDI DIN-5 Serial Cables

This reference information in case you have to work with old school serial MIDI.

__MIDI DIN-5 connector pinout__

|PIN    |  MIDI IN        | MIDI OUT         | MIDI THRU
----    |-----            |------            |-----
1       | NC or GND       | NC or GND        | NC or GND
2       | SHIELD          | SHIELD           | SHIELD
3       | NC or +V        | NC or +V         | NC or +V
4       | MIDI source IN  | MIDI source OUT  | MIDI source OUT    (+5V)
5       | MIDI sink OUT   | MIDI sink IN     | MIDI sink IN

The actively driven line is always the MIDI current sink (Pin 5).
The MIDI current source is just a +5V supply through a 220 Ohm pullup resistor.
On the MIDI IN connector, the data signal on pins 4/5 is usually hooked to an optoisolator.

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
