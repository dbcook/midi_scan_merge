# MIDI Shields for Arduino

I obtained a few copies of an inexpensive generic Arduino MIDI shield ($5 ea. on Ebay, shipped from China).

## Generic Shield

The generic shield is extremely simple, basically being an adaptation to put MIDI connectors and an input opto-isolator
on RX0 / TX0 (Arduino device name "Serial").  The use of SER0 is established in hardware and is NOT configurable.
Many listings for the same exact board design can be found on eBay, AliExpress, and elsewhere.

The board hard MIDI In, Out and Thru connectors.  It's essentially certain that the Thru connector is a hardwired
mirror of the In port, since there are no traces on the board for any of the other serial ports.  The In and Out
ports are just wired to the two sides of the SER0 UART.

The only auxiliary functionality on the card is a mirrored RESET button (S1), and a PROG/RUN switch (S2) that disconnects
the MIDI circuitry from SER0 when you are loading code into the Arduino, since that uses SER0.

This shield is plug-n-go and requires no assembly.

### No Stacking Above

There are no upper headers for stacking additional shields above this one, so it has to be the top one
in any stack, e.g. with the Ethernet shield underneath it.  The MIDI connectors are rather tall, so the normal
upper header strips would be too short by about half.  You also cannot use multiple instances of this board
anyway due to the nonconfigurable serial port; the signals would conflict.

### Prototyping Area

The MIDI shield has a small prototyping area at one edge consisting of plated through holes on header spacing.
Signals D2 to D13 and A0 to A5 are brought here, as well as 18 ground and 6 power positions.  LEDs can be
directly mounted here and other things can be adapted via headers.  This feature ensures that the signals
physically occupied by the shield's lower headers remain accessible in some form.

### Usability Issue

With this shield you MUST manually turn off the PROG/RUN switch S2 each and every time you load code into the Arduino,
and turn it back on again afterward.  (The OFF position should really be labeled PROG).
This is a major PITA because it prevents any easy setup for remote or fully automated code loading.
Using SER0 that's shared with the bootloader is a really bad idea, except that on some Arduini
that's all you've got.

That's another reason I'm leaning toward doing everything on Ethernet RTP-MIDI.

### Hack Remedy for the PROG/RUN problem

When using an Arduino Mega 2560 with its four hardware serial ports, it would be an easy hack to
attach this board to a different port:

1.   Clip off the D0 and D1 pins (RX0 and TX0) from the shield's header pins
1.   Get a pair of header leads with plugs on one end, about 3" long.
1.   Plug one header lead into D14 (TX3) and solder the other lead to the cut-off pin for D1
1.   Plug the other header lead into D15 (RX3) and solder the other lead to the cut-off D0 pin.
1.   In the software, open the serial port on Serial3 instead of Serial.

This gives up 2 digital I/Os but totally eliminates the need to use the PROG/RUN switch.

### GPIO Ports Used

D0 = RX0. Connected to the MIDI In connector via an optoisolator (swtiched off via the slide switch S2)
D1 = TX0. Connected to the MIDI Out connector.

## SparkFun Configurable Shield

The [SparkFun MIDI Shield Kit](https://www.sparkfun.com/sparkfun-midi-shield.html) can be configured to use a
software UART instead of hardware SER0 and has various extras including 2 pots on A1 and A2, 3 pushbuttons on
D2, D3 and D4, and 2 red+green status LEDs on D6 and D7.  

The ability to use a software UART is very nice because it eliminates the need to flip the PROG/RUN
switch every time you load code.

The price for these advantages is that it costs $24.50 + tax which is 4x more than the generic board.
It's furnished in kit form so that unneeded LEDs, pots and buttons can be left unpopulated, which is a must
if those signals are to remain available for other shields.

This board does not include the stackable headers in the kit; they would have to be added at the time you build the board.
The SparkFun [hookup guide](https://learn.sparkfun.com/tutorials/midi-shield-hookup-guide) has more info about this.

The SparkFun shield does not have a 3rd connector for MIDI Thru, so if you want Thru functionality you
either have to do it in software or rewire the Out connector to be a Thru, which sacrifices the ability
of the Arduino to send MIDI messages.  If you use the 47effects library, it's trivial to implement MIDI thru
in software, so I don't see the lack of a 3rd connector as a problem.  A hardwired thru port that can't do
merging with Arduino-generated messages seems not very useful.

On the SparkFun MIDI shield, the pushbutton on D4 conflicts with the SD card chip select on the Ethernet Shield, so
in such a stack, the D4 button would not be available (and should not be populated) as D4 has to be configured as an output
for the Ethernet shield to work properly.

## References

[Arduino forum thread with hardware info for the generic shield](https://forum.arduino.cc/t/midi-shield-breakout-box-free-pins/1336926/5)