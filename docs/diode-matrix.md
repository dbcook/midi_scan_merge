# Diode Matrix Notes

## Nektar GX61 Probing

Here are notes from Issue #15 - trying to see what likely matrix arrangements and connectors are for normal MIDI controllers.

The Nektar GX61 is a basic but well-regarded unit with velocity sensing (meaning double-contact keys).
It has almost all-plastic construction apart from fasteners.  Key touchweight is noticeably stiffer than
my Laukhuff wood keyboards but not as stiff as Fatar.

### Physical Construction

The unit has almost all-plastic construction apart from fasteners.  Key touchweight is noticeably stiffer than
my Laukhuff wood keyboards but not as stiff as Fatar.

Disassembly is simple - all you have to do is flip the keyboard on its back and remove all of the visible screws.
The top and bottom parts are hinged together at the left end by two rather short ribbbon cables.
Care is needed when disconnecting them.

### Electrical Construction

Internally there are two ~16" long PCBs - spanning 32 and 29 notes respectively - under the keys with
printed-on contact bridges. The two PCBs are connected by two 8-conductor hardwired umbilicals.

The on-PCB plated contacts are coated with a darker material that adds  3-4 Ohms of resistance. 
That's only 2% of the 220 Ohm pullup value so won't noticeably affect the RC time constant for the inputs.

Each PCB carries two full and one partial 12-note Fatar-style silicone rubber strips with two conductive pills
at staggered heights at each key position.

At the end of the larger PCB is a pair of connectors that go to the MCU boards.  The connectors are 14 and 13 position
so that they can't be plugged into the wrong slot.  There are 26 conductors, 13 on each connector; the 14-pin
connector has one no-connect position.  That's two more than the 24 needed for an 8x16 matrix so we need to
figure out why the other two are there.  One possibility is a contact closure presence detection.

Inspection of the PCBs shows no components other than diodes and connectors, meaning the matrix is completely
passive and should not be sensitive to the MCU pullup voltage.

### Diode Matrix Circuit Details

[General matrix layout](images/Nektar%20GX61%20Diode%20Matrix%20Layout.jpg)

Note - the original diagram was done on SmartDraw, which although quick is a buggy mess that I won't use again.
Need to redo in draw.io with full details on the pinout of the main connectors and the umbilicals.

* The matrix is a straight 8 select x 16 read.
* The read lines are hooked directly to one side of the contact pads.
* The reads alternate between blocks of 8 notes on the two sets of contacts.
* On the other side, the negative (banded) end of both diodes for a given note are connected to a select line.
  Every 8th note is connected to the same select line all the way across both PCBs.
* The "north" umbilical has the 8 read lines going to the smaller board
* The "south" umbilical carries the 8 select lines, each of which goes to a group of diode pairs spaced
  8 notes apart on the 29-note PCB. These are the select line outputs from the MCU.

What remains:

* Probe the remaining read pins to get the full sequence across both PCBs and generate the full pinout for the two connectors on the large PCB.
* Figure out why there are two more conductors on the connectors than needed for an 8x16 matrix.
* Find matching connectors so that we can make adapter cables to hook those up to the Arduino for testing.
* Find a good/efficient way of plugging the rubber strips back into the holes on the PCBs. 1mm metal pin works but requires extra care to not poke through the rubber.