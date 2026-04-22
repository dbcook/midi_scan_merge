# Arduino Ethernet Shield v2 Info

Documented as compatible with Arduino Uno and Mega. The shield requires an Arduino that has the ICSP connector.

## GPIO Pins Summary

### Pins Used for Standard Arduino Functions

| Pin    | Description
|---     | ---
| D0     | RX0 1st hardware serial port "Serial" - used by bootloader and debug console
| D1     | TX0 1st hardware serial port "Serial" - used by bootloader and debug console
| D13    | Arduino LED "L"

D0 and D1 (RX0 and TX0 serial) are needed for code loading.

### Pins Used by Arduino Mega 2560 Specific Functions of Interest

Mega 2560 has 3 additional hardware serial ports.  These can be used as GPIO if not configured
to be serial ports.

| Pin    | Description
| ---     | ---
| D14    | TX3 for hardware Serial3
| D15    | RX3 for hardware Serial3
| D16    | TX2 for hardware Serial2
| D17    | RX2 for hardware Serial2
| D18    | TX1 for hardware Serial1
| D19    | RX1 for hardware Serial1

### Pins Used by Ethernet Shield

The Ethernet Shield 2 consumes only the following GPIO pins:

* D4 : chip select for the SD card
* D10 : select for the WizNet 5500 chip
* D13 : drives a mirror of the standard Arduino general purpose LED labeled "L"

### Mechanical Access

The clear plastic carrier sled for the Ethernet Shield 2 blocks easy physical access to:

* D14 = TX3 serial
* A6
* A7

You can regain access to D14 and A7 of by using the shield without the plastic carrier, or by cutting away
a little of the sled (tested and works OK).  This will *not* work for A6 and you will need to create a bit
of custom wiring for that since the PCB of the Ethernet shield shadows it.

### Available Pins when Ethernet Shield 2 is Used with the Mega 2560

| Pin Range | Num Pins. | Alt. Functions
| ----      | ----      | ---- 
| A0 - A15  | 16        | Analog I/O
| D2 - D3   | 2         |
| D4        | ---       | chip select for SD card
| D5 - D9   | 5         |
| D10       | ---       | chip select for WizNet 5500 Ethernet chip
| D11 - D12 | 2         |
| D13       | ---       | LED "L"
| D14 - D19 | 6         | TX3, RX3, TX2, RX2, TX1, RX1 serial
| D20 - D21 | 2         | SDA, SCL; or TWI interface connectors
| D22 - D53 | 32        |

Thus if we do not use the MIDI shield at all, we have available on the Mega 2560:

* 49 digital pins
* 16 analog pins

That is enough for *four* 8x8 diode matrix keyboards, or a 65 pin parallel scanner.

If we use the MIDI shield with the hardware hack to rewire it away from SER0 to (say) SER3, we lose
D14 and D15 as well as A6, leaving only 46 digital pins, two shy of handling three 8x8 diode matrix
scanners on the digital pins.

We could get back to 65 pins by using the MIDI shield without the serial remapping hack, but this
would require PROG / RUN switching when loading code and would prevent console debug messages.
Console messages would need to be completely turned off in the code as well, so that the debug
facility neither intializes nor tries to use RX0/TX0.  Capability for this has already been
implemented in the code.

## GPIO Pins Used: Details

The following pin routings connect some Arduino lines to the white and orange I/O connectors.
There is no digital logic of any kind on those signals, so if you don't attach anything
to the connectors they remain available as GPIOs.

* A2 - input on white IN2 connector (3 pin)
* A3 - input on white IN3 connector (3 pin)
* D5 - output on orange OUT5 connector (3 pin)
* D6 - output on orange OUT6 connector (3 pin)

* SCL - appears on pin 4 of both green TWI (two wire interface) connectors (4-pin)
* SDA - appears on pin 3 of both green TWI connectors

The D4 and D10 pins are used by the Ethernet shield as chip select lines and cannot
be reclaimed for GPIO use whenever the Ethernet shield is present:

* D4 = CS chip select for SD card
* D10 - selects the WizNet card

The default Arduino LED is on D13 in the Mega 2560.  It could be used as an output but
probably not as an input (check this).

* D13 - the Ethernet shield drives a mirror of the Mega onboard LED "L".  This LED is immediately
adjacent to the right side of the Ethernet connector on the shield.  The next LED to the right is
a power indicator and is not programmable.

The SPI interface is implemented via the separate ICSP header in the middle of the shield v2 and mega boards.

In documentation for the original Ethernet shield, D10-12 are described as implementing the SPI connection to the WizNet chip.
With the advent of the ICSP header, those lines are freed up for use by the Mega.

Ethernet LEDS - activity, FDX and link LEDS are driven directly from the WizNet chip

Power In - 5V
Power Out - regulates +5V to +3V3 on IC2 - local or exported?
