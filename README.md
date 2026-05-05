# Flexible MIDI Scanner - Merger - Decoder for Arduinos

This PlatformIO based project targets Arduino family processors with large digital IO pin counts,
and is aimed at exploiting the number of IO pins in order to scan up to four 8x8 diode matrix groups
representing up to 256 discrete contacts.

We perform high precision switch debouncing and emit MIDI NoteOn and NoteOff messages accordingly.

The system context for this is to support musical instruments such as pipe organs - actual or virtual - that
may have a large number of inputs:  four or more 61-note keyboards, a 32-note pedalboard, up to 100 momentary-contact
thumb buttons ("pistons" in traditional organ lingo), up to 25-30 toe pistons, and 5 analog expression pedals ("shoes").
These instruments can approach 500 total inputs with single-contact keyboards and 800 with dual-contact
velocity sensing keyboards.  Thus in all cases multiple scanner/encoders will be needed, so maximizing
their capacity is useful to control cost and system interconnects.

## State of the Project

The Ethernet Shield 2 is installed on the test rig and intial Ethernet library integration is
largely done. USB MIDI cannot be tested on a Mega 2560 since it does not support a full USB host. That
will require an Arduino Due or Teensy 4.1.  A Due is on order and I have a couple of Teensy 4.1
units.  Current work is focused on getting NV param storage running cross-architecture on both
the Mega 2560 and the Due (occasionally referred to by its full name of Duemilanove).

Arduino Due uses a different architecture (SAM vs AVR) so there is a separate config for it
in `platformio.ini`.  Right now both configs compile but proper operation is not confirmed.

Originally a prototype was implemented that seemed to run quite quickly on the Mega 2560.  However the
straightforward code design ended up using 35-40% too much memory and would only have supported two
8x8 and one 8x4 diode matrix groups.

A second version was built that reduced RAM usage by enough to get
the data for a full slate of four 8x8 diode matrix arrays into about 4KB of RAM.  The time per
input scanned was about 9-12 usec depending on circumstances.  This version passed several
logging tests to prove that the pin block definitions are being read and scanned properly.
Several matrix and block scenarios appear to run correctly.  Testing with actual input
switches is pending.  Thus far all of the debugging has used a generic serial MIDI
shield with the hardware hacked to attach it to the Mega 2560 SER3 port.

After discovery that the stock Arduino digital IO library routines were eating up an
excessive amount of CPU, a third version has been implemented that uses the digitalWriteFast
libary to drastically reduce the IO overhead and give a scan time per input of about 6 usec
on the Mega 2560.

Since the 3rd prototype, the Ethernet libraries have been integrated up to the point where
session establishment and note emitting code is in place.  Functional tests are pending.

### Current Hotspot

There is a significant issue with getting portable NV param storage.  The Mega and Due use
processors with different architecture and features.  Notably the Mega has 4 KB of onboard
EEPROM while the Due has none.  It is possible to write flash pages at runtime on the Due
but not on the Mega.

EEPROM.h param storage has been the Arduino standard from the early days, but it can only
be used for Arduino Mega 2560 with its 4K built-in EEPROM.
For Arduino Due there is a Due Flash Storage Library https://github.com/sebnil/DueFlashStorage.
Unfortunately the Due Flash Storage library is very primitive and does not even support a multi-byte read.
It also has not been updated in a long time, but is widely reported to work.

#### NV Params Options

__Option 1__ Store all params on the SD card on the Ethernet shield

Pros: hopefully no arch dependent code, possible to use yaml or ini format, can configure externally with a PC
Cons: not all rigs will have the eth shield, minor cost for SD cards

__Option 2__ Fork DueFlashStorage and bring it up to parity with EEPROM.h by adding
* word, long, and arbitrary length read
* word and long writes (note that any multi-byte write must be on 4-byte boundaries)
* update versions of all writes (compare with existing flash before writing)

Pros: no eth shield needed

Cons: messy work to wrapperize 2 disparate libs and enhance one of them a lot, byte alignment



## Contributing

*Bug reports* Please use GitHub issues in the normal way.

*Fixes and enhancements* Plase follow the conventional open source process:  fork this repo,
create a branch with your changes on it, and submit a pull request (PR) for that branch. Be sure
to generally follow the conventions used in this project; nonconforming PRs will not be accepted.

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

* Programmable pinBlock parameters (including MIDI output channels) via SYSEX messages with readback
* Velocity sense (dual contact) scanning
* Aftertouch (triple contact) scanning
* Support IO extenders
* MIDI decoder - consume messages on one or more channels and activate outputs
    * LEDs
    * Multi-digit displays (e.g. sequence frame number)
    * Solenoid drivers (e.g. for electromechanical organ stops)
* Teensy support
* Hardware
    * General purpose carrier and interconnect board for Teensy 4.1
    * Diode matrix boards for parallel keyboards
    * LED driver (matrix based) for MIDI decoder

## Target Processors

There are at least 3 suitable processors in the Arduino family with high pin counts and adequate speed.
This table summarizes their capabilities and cost with Ethernet capability added.  The two
Arduinos cost $50 but both need an Ethernet Shield ($30).  The Teensy 4.1 costs $31.50 with onboard Ethernet
PHY and only needs a MagJack connector and a carrier board (est. $10), making it much cheaper and
vastly more capable than the Arduinos.

| Processor     | RAM       | Flash     | Speed     | Dig Pins  | Usable | USBHost | Cost | Remarks
| ----          |----       |----       |----       |----       |----    |----     |----  |----
| Ard Mega 2560 | 8 KB      | 256 KB    | 16 MHz    | 69        | 65     | NO      | 50   | Barely fast enough, eth $30
| Arduino Due   | 96 KB     | 512 KB    | 84 MHz    | 69        | 65     | YES     | 50   | Fast enough, eth $30
| Ard Leonardo  | 2.5 KB    | 32 KB     | 16 MHz    | 20        | 18     | YES     | 24   | Low pin count, small RAM
| Teensy 4.1    | 1 MB      | 7936 KB   | 600 MHz   | 55 + 18   | TBD    | YES     | 42   | Max Overkill

If you are committed to using Arduinos and don't want to mess with making carrier boards, the Due
is the best choice.  It is a proper superset of the Mega 2560 at the same price, but adds 5x higher
speed, USB host capability, 2x more flash, and 8x more RAM.  Because of this it's likely that I will
drop support for the Mega 2560 in future releases.

The Leonardo offers USB host capability and is half the cost of the Due, but has only 1/3 the IO pins
and a really tiny amount of RAM, and is 5x slower.  At most it can handle one 8x8 keyboard matrix.

### Arduino Architecture Differences

The Mega 2560 uses the Atmel AVR architecture, while the Due uses a SAM chip (SAM3X8E).  The Arduino
boards for these have quite similar capabilities, but there are differences that have to be taken into account:

* The Mega 2560 AVR processor has 4K of built-in EEPROM, while the Due does not, and uses a page of flash for NV
storage instead.  This means that different NV libraries must be used, and the APIS, while similar,
are not completely compatible.
* The AVR will accept 5V external inputs, while the Due can only take 3.3V.  This doesn't matter
for typical key/pedal scanners because the contact systems are completely passive, but you have
to be careful about hooking up externally sourced active-high inputs.
* The Mega 2560 has 16 of its 69 IO pins configurable as analog, while the Due only has 12 out of 69.
This is because the Due provides different aux functionality on pins 50-53, including two DACs.
* The `MemoryFree` library used to measure memory consumption only works on the AVR architecture.
For SAM the heap boundary symbols are different.  For now this is of no consequence since it doesn't
look like memory will be an issue on the Due. There is [code suitable for SAM here]()

## Prerequisites and Building

To configure, build and load this software into an Arduino, you need vscode with the PlatformIO plugin.

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

Switch debounce is performed for both the "make" and "break" actions.  20 millisec is usually considered sufficient.

### Performance

The scan cycle rate is accurately measured using the Arduino millisecond timer and emitted on the serial
console port at 1 Hz, allowing the effects of algorithm changes to be seen immediately.

The initial version of this package runs the scan loop for a single 8x8 matrix (64 inputs, 1 typical keyboard)
at about 1.5 kHz on a Mega 2560, without the benefit of the new fastread IO.  Fastread is expected
to remove about 4 microsec per input read (currently 10.0-10.8 depending on which pins are being read),
which should immediately improve the single 8x8 scan rate to around 2.5 KHz.  On a linear assumption
this would mean two 8x8's would scan at 1.25 KHz and three 8x8's at 0.9 KHz. All of these rates should be
adequate to keep scan latency from ever being noticeable.

### MIDI Merge and Daisy Chaining

If a serial MIDI shield is used, the software can be configured to perform a merge operation, where messages
arriving on the serial MIDI link are sent onward over the output transport link.  MIDI channel separation remapping
is implemented if that is needed for merging inputs from multiple scanners that are emitting notes on the same
channel.  

It is possible for a few of these Arduino scanners to be daisy-chained by configuring for both serial
output and input.  
Multiple serial inputs are supported in the software, though the inexpensive
MIDI serial shields cannot be stacked without some hardware hacking.
But daisy chaining is not a good idea from a latency perspective;
chaining of serial MIDI ports introduces 2+ msec MINIMUM of delay per hop, so single notes
from the most distant manual in a 4-manual daisy chain could be delayed by as much as 10 msec.
The last note in a big chord from the end of the chain could see 20-25 msec of latency.


## Constraints

Overall there is enough memory and enough CPU to scan and debounce 192-448 inputs in up to seven 8x8
diode matrix groups, depending on the speed and memory of your MCU. Even the slow 16 MHz
processors should be able to do 3 8x8x matrix groups.  On the Mega number of matrix inputs
that can be scanned is both RAM and CPU limited, and not IO pin limited.

If parallel inputs are used, you can only scan 64 total inputs, so memory and speed will
not be a problem on any Arduino or Teensy MCU.  In this case you are completely pin limited
and an IO expander could be very helpful.

### Memory

Nearly all RAM is statically allocated to avoid problems with unpredictable heap usage.
The build output in the terminal will tell you how much of the static RAM and flash are used.

Both the Leonardo (2.5 KB) and Mega 2560 (8 KB) have plenty of flash but very limited RAM.
In an attempt to make the Mega reasonably useful, some code complexity has been undertaken to not unpack
the pin block definitions into RAM, and to use index based access of
debouncer memory structures from a statically defined array.  The latter by itself saves 1KB
of RAM for 256 inputs.

The debouncers as currently implemented use 14 bytes of memory each, so 256
of them use 3.6K of RAM.

Given that the Arduino Due is virtually identical to the Mega 2560 (even in cost), I strongly
suggest that you should use the Due instead of the Mega 2560, especially as I may drop support for the
Mega 2560 to get rid of some of the code that was necessary to make the Mega 2560 usable.

### CPU

The 16MHz Mega 2560 takes (with fastread IO) about 5-6 microseconds per input scanned, depending on whether they are in a
diode matrix where the select pins have to be written or a parallel block without that overhead.
This means that the scan time for 3 61-note 8x8 matrix blocks (183 inputs) should be about 1 to 1.2 milliseconds.

The Arduino Due is 5x faster and the Teensy 4.1 is 50x faster, so CPU considerations for these are currently nil.

### IO Capacity

Both the Mega and Due have 69 total IO pins, all of which can be programmed as digital inputs or outputs.
Out of the set of 69 pins, various subsets can be programmed for other functions such as 
analog input, serial ports, PWM output, and some specialty functions.  Most IO shields
will consume a few pins.  On a Mega or Due with an Ethernet Shield 2 installed, the
following 5 pins are not usable, leaving exactly 64 usable digital IO pins:

* Pins 0 and 1:  the base serial port used by the bootloader
* Pins 4 and 10: used as chip selects by the Ethernet shield
* Pin 13: the standard Arduino LED output

Thus With the Ethernet shield attached, on the Mega 2560 and Due you have exactly 64 usable IO pins,
of which 16 (Mega) or 12 (Due) are analog-capable.
This will physically handle 4 full 8x8 diode matrix keyboards (without using shared scan lines).
or up to seven 8x8's if the same scan lines are used for each matrix.  Using one or more of
the analog-capable pins for analog inputs OR using any digital pins for parallel inputs will
reduce by one the number of 8x8 diode matrix groups that can be supported.

If a MIDI serial shield is used and you do not want to have to flip PROG/RUN switches everytime you load code,
you have to give up two more IO pins, leaving only 63 usable if an Ethernet shield is also present.
This is a good reason not to have a serial MIDI shield attached unless it is actually used.
See [docs/midi-shields.md](docs/midi-shields.md) for specifics on wiring serial MIDI shields.

The largest number of 8x8 diode matrix groups that can be scanned with 64 pins is seven,
provided that common scan outputs are used for all of the matrix groups.  Each independent
set of scan lines consumes 8 pins.  Thus if you use independent scan lines for each matrix,
you can only do 3 8x8 groups.

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
read functions).  This is crucial on the slow Mega 2560.

In order for digitalReadFast() to work the pin number has to be a compile time constant.  Thus to use this with our
flexible pin block definition scheme we need a function that resolves a pin number in a variable to
the specific digitalReadFast macro for that pin.  That has been accomplished using an array of
function pointers stored in flash and a simple wrapper function that invokes the correct macro.
This adds a single-digit number of clocks per invocation but is still about 10x faster than
using digitalRead().

An implementation using whole-byte reads of the CPU ports was considered but not adopted, because
the mapping of IO pins to the CPU ports is irregular and it doesn't look like the additional
reduction in clocks per IO read would be enough to matter.

A theoretical further speedup would be to write code for predefined layouts of common pin blocks (8x8 and 
4x8 diode arrays, plus 32 pin parallel) using fixed pin assignments for the select and read lines.
This could explicitly use digitalReadFast and digitialWriteFast with no additional overhead, but 
would be very hard to maintain because the loops would have to be completely unrolled.


### Velocity and Aftertouch Sensing

Velocity sensing for keyboards (not yet implemented) will double the number of contacts and will
double or nearly double the memory consumption of each debouncer.  It will also slow down the scanning slightly
due to increased complexity of the algorithm while a key has at least one contact closed.
However I think it should be possible to do velocity sensing on a 61-note keyboard with a single
Mega 2560 at better than 1 KHz.

Aftertouch sensing, used on theater organs and synthesizers, involves 3 contacts per key.
Implementing it just means stacking a 3rd layer of debounce atop the velocity sensing logic.
Because of the number of IO pins needed, it will consume 3 8x8 matrix slots.  Thus without
sharing scan lines or an IO extender you will only be able to scan one such keyboard per Arduino.

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
