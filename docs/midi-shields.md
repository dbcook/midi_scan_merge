# MIDI Shields for Arduino

I obtained a few copies of an inexpensive generic Arduino MIDI shield ($5 ea. on Ebay, shipped from China).

## Serial MIDI General Info

### Data Rate and Message Throughput
The classic MIDI serial interface (on the ubiquitous DIN-5 connectors) data rate is only 31250 baud, which is extremely slow
by modern standards and is not suitable for situations with a high message rate.

MIDI NoteOn messages are 3 bytes, so you can get a max ~1KHz msg rate on the link. The protocol officially allows for
2-byte "running status" msgs, but that is not widely supported.

### Daisy Chaining and Differential Message Arrival Times

It's fairly common to see daisy-chaining of serial MIDI interfaces, which adds a minimum of 2 msec of latency per hop
due to the wire speed and probably more like 3 msec due to processing time for the relay in firmware.
Thus if 5 serial MIDI links are chained in an organ system, there will be 4 hops adding 12 milliseconds of raw latency.

That would be acceptable if only sporadic single notes occur.  However, when several notes are generated at once or
nearly at once (think a big chord with 4-5 notes in each hand) on multiple nodes in the chain, the aggregate delay
and arrival time spread between the notes becomes noticeable.  In the worst case 2-handed chord scenario with 4 hops
in the chain, each hand's chord will take 5 msec to be initially transmitted and there will be 12 milliseconds of
daisy-chain hop delay between the first chord group and the second.  This amount of latency would not be an issue
if it was consistent, but the total spread between the first and last notes' arrival at the destination will be
at least 22 milliseconds, and it will not always be the same depending on which keyboards were used.

An even more problematic scenario is where as many as 100 messages may be sent simultaneously to reconfigure a group
of organ stops or send a batch of note-off messages for clearing a bunch of LEDs.
Here the string of messages could take over 100 milliseconds in transit, so the arrival time spread will be
very noticeable.  If notes were being sent over the same link there will be noticeable raggedness.

Overall this makes a very strong case for using transports like USB-MIDI and Ethernet RTP-MIDI (aka AppleMidi)
that are hundreds to thousands of times faster and make differential arrival time issues go away.

## Generic MIDI Shield

The generic MIDI shield is extremely simple, basically being an adaptation to put MIDI connectors and an input opto-isolator
on RX0 / TX0 (Arduino device name "Serial").  The use of SER0 is established in the shield's hardware and is NOT configurable in software
but can (and should) be hacked as described below to avoid using SER0.
Many listings for the same exact board design can be found on eBay, AliExpress, and elsewhere.

The board has MIDI In, Out and Thru connectors.
The MIDI In and Out connectors are just wired to RX/TX of the SER0 UART on the Arduino.
The Thru connector is a hardwired mirror of the In port as per the original MIDI reference design.
The hardware THRU connection is useful as a zero-latency relay for a device that only consumes messages,
but is useless on an input scanner that generates MIDI messages itself and must add them to the message stream.

The only auxiliary functionality on the card is a mirrored RESET button (S1), and a PROG/RUN switch (S2) that disconnects
the MIDI circuitry from SER0 when you are loading code into the Arduino, since the bootloader also uses SER0.

This shield is plug-n-go and requires no assembly, but needs a modification to change the Arduino serial 
port used (see below).

### No Stacking Above

There are no upper headers for stacking additional shields above this one, so it has to be the top one
in any stack, e.g. with the Ethernet shield underneath it.  The MIDI connectors are rather tall, so the normal
upper header strips would be too short by about half.

You could easily use multiple MIDI shields with an Arduino Due or Mega without stacking.  The MIDI shields should only
need +5V, GND and Serial TX/RX, so you could remove the headers and use 4-pin cables to connect them to the Arduino.
But overall if you want multiple MIDI input ports to make a serial to Ethernet or USB merger, I'd just make a
little PCB with only the needed MIDI connectors and opto-isolators and route them to UARTs on the Due/Mega.

### Prototyping Area

The MIDI shield has a small prototyping area at one edge consisting of plated through holes on header spacing.
Signals D2 to D13 and A0 to A5 are brought here, as well as 18 ground and 6 power positions.  LEDs can be
directly mounted here and other things can be adapted via headers.  This feature ensures that the signals
physically occupied by the shield's lower headers remain accessible in some form.

### Usability Issue or Why There's a Switch on the MIDI Shield

With this shield, unless you modify it as described below to use a different serial port other than SER0
on the Arduino, you MUST manually turn off the PROG/RUN switch S2 every time you load code into the Arduino,
and turn it back on again afterward.  (The OFF position should really be labeled PROG).
The switch is required because the MIDI shield is wired to use Arduino pins 0 and 1, which
are shared with the bootloader and debug console.

Using the switch is a PITA because it prevents an easy setup for remote or fully automated code loading.
Using SER0 that's shared with the bootloader is a really bad idea, except that on some Arduino boards
that's all you've got.

### Remedy for the PROG/RUN Problem

When using an Arduino Due or Mega 2560 with their four hardware serial ports, it's an easy hack to
attach this board to a different hardware serial port, eliminating the need for switch-flipping
when loading code:

1.   Clip off the D0 and D1 pins (RX0 and TX0) from the shield's header pins
1.   Get a pair of header leads with plugs on one end, about 3" long.
1.   Plug one header lead into D14 (TX3) and solder the other lead to the cut-off pin for D1
1.   Plug the other header lead into D15 (RX3) and solder the other lead to the cut-off D0 pin.
1.   In the software, open the MIDI serial port on Serial3 instead of Serial.

This gives up 2 digital I/Os but totally eliminates the need to use the PROG/RUN switch.
I did this to one of my boards - only took 5 minutes and it works perfectly.

### GPIO Ports Used

In the shield's default configuration:

D0 = RX0. Connected to the MIDI In connector via an optoisolator (swtiched off via the slide switch S2)
D1 = TX0. Connected to the MIDI Out connector.

If you make the modification described above, these pins will no longer be used in favor of whichever
higher serial port you chose.

## SparkFun Configurable MIDI Shield Kit

The [SparkFun MIDI Shield Kit](https://www.sparkfun.com/sparkfun-midi-shield.html) can be configured to use a
software UART instead of hardware SER0 and has various extra features including 2 pots on A1 and A2, 3 pushbuttons on
D2, D3 and D4, and 2 red+green status LEDs on D6 and D7.  

The ability to use a software UART is nice because it eliminates the need to flip the PROG/RUN
switch every time you load code.  However the software UART consumes more CPU than hardware UART, so
MIDI traffic becomes more of a load on the processor.

The price for these advantages is that it costs $24.50 + tax which is 4x more than the generic board,
and you have to spend time building it.
It's furnished in kit form so that unneeded LEDs, pots and buttons can be left unpopulated, which is a must
if those signals need to remain available for other shields.

This board does not include the stackable headers in the kit; they would have to be added at the time you build the board.
The SparkFun [hookup guide](https://learn.sparkfun.com/tutorials/midi-shield-hookup-guide) has more info about this.

The SparkFun shield does not have a 3rd connector for a hardware MIDI Thru. If you use the 47effects MIDI library,
it's trivial to implement MIDI thru in software, so I don't see the lack of a 3rd connector as a problem.

On the SparkFun MIDI shield, the pushbutton on D4 conflicts with the SD card chip select on the Ethernet Shield, so
in such a stack, the D4 button would not be available (and should not be populated) as D4 has to be configured as an output
for the Ethernet shield to work properly.

## References

[Arduino forum thread with hardware info for the generic shield](https://forum.arduino.cc/t/midi-shield-breakout-box-free-pins/1336926/5)