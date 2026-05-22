# Keyboard Diode Matrix Info

When you look for keyboard diode matrix material, you find a lot of things about mechanical computer keyboards, which
seems to be a much hotter area than musical instrument keyboards for DIYers.

Thus far I've not come up with any open source standalone parallel-to-matrix PCBs.  One reason seems to be that they
are so easy to make that everyone does their own.

There is sometimes a preference to hold the number of read pins at 8 and construct the matrix with possibly larger numbers
of scan pins.  This is because for some MCUs you can map the 8 read pins to a register/port and read that port all at once.

This could work on most or all Arduino processors that have at least one IO Port that maps to preferably consecutive
pins.  Up to now I have preferred to have more read pins than select pins since that reduces the number of select pin writes
when reading one input at a time.

However the faster Due has a completely different port architecture with the pins scattered across four 32-bit ports.
Speed tests on reading all 4 ports are pending.
Reading specific pins will take either a big switch statement or index function pointers so it's not clear that
things will be much better than fastread().

## Port Mapping for Arduino Processors

### Mega 2560

Here is a summary of the Mega 2560 mapping of pins to 8-bit ports.

 PORTE       0-3     5
 PORTB       10-13   50-53
 PORTJ       14-15
 PORTH       16-17   6-9     7-9 are reverse bit order on bits 4-6, pin 6 is bit 3 so it goes 6987
 PORTD       18-21   38      pretty much random bit placement 18 = bit 3, pin 19 = bit 2, 20 = bit 1, 21 = bit 0, pin 38 = bit 7
 PORTA       22-29
 PORTC       30-37           reverse bit order
 PORTG       39-41   4       reverse bit order
 PORTL       42-49           reverse bit order
 PORTK       62-69           (analog A8-A15)
 PORTF       54-61           (analog A0-A7)

 ### Due

 The Arduino Due uses four 32-bit ports designated PORTA, PORTB, PORTC, and PORTD.  The pin to port assignments are all
 scattered.

 Here is the Due mapping in order of conventional Arduino IO pin number:

| Due Pin  | Port-pin designator
|----      | ----
| 0        | PA8
| 1        | PA9
| 2        | PB25
| 3        | PC28
| 4        | PA29
| 5        | PC25
| 6        | PC24
| 7        | PC23
| 8        | PC22
| 9        | PC21
| 10       | PA28
| 11       | PD7
| 12       | PD8
| 13       | PB27
| 14       | PD4
| 15       | PD5
| 16       | PA13
| 17       | PA12
| 18       | PA11
| 19       | PA10
| 20       | PB12 alt PA17
| 21       | PB13 alt PA18
| 22       | PB26
| 23       | PA14
| 24       | PA15
| 25       | PD0
| 26       | PD1
| 27       | PD2
| 28       | PD3
| 29       | PD6
| 30       | PD9
| 31       | PA7
| 32       | PD10
| 33       | PC1
| 34       | PC2
| 35       | PC3
| 36       | PC4
| 37       | PC5
| 38       | PC6
| 39       | PC7
| 40       | PC8
| 41       | PC9
| 42       | PA19
| 43       | PA20
| 44       | PC19
| 45       | PC18
| 46       | PC17
| 47       | PC16
| 48       | PC15
| 49       | PC14
| 50       | PC13
| 51       | PC12
| 52       | PB21
| 53       | PB14
| 54  A0   | PA16
| 55  A1   | PA24
| 56  A2   | PA23
| 57  A3   | PA22
| 58  A4   | PA6
| 59  A5   | PA4
| 60  A6   | PA3
| 61  A7   | PA2
| 62  A8   | PB17
| 63  A9   | PB18
| 64  A10  | PB19
| 65  A11  | PB20
| 66  A12  | PB15
| 67  A13  | PB16
| 68  A14  | PA1
| 69  A15  | PA0


Here is the same table in order of Port-pin designator

| Due Pin  | Port-pin designator
|----      | ----
| 69  A15  | PA0
| 68  A14  | PA1
| 61  A7   | PA2
| 60  A6   | PA3
| 59  A5   | PA4
| -- 1 bit skipped --
| 58  A4   | PA6
| 31       | PA7
| 0        | PA8
| 1        | PA9
| 19       | PA10
| 18       | PA11
| 17       | PA12
| 16       | PA13
| 23       | PA14
| 24       | PA15
| 54  A0   | PA16
| 20       | PA17 alt PB12
| 21       | PA18 alt PB13
| 42       | PA19
| 43       | PA20
| -- 1 bit skipped --
| 57  A3   | PA22
| 56  A2   | PA23
| 55  A1   | PA24
| -- 3 bits skipped --
| 10       | PA28
| 4        | PA29
| -- 3 bits skipped in PORTA, 12 bits skipped in PORTB --
| 20       | PB12 alt PA17
| 21       | PB13 alt PA18
| 53       | PB14
| 66  A12  | PB15
| 67  A13  | PB16
| 62  A8   | PB17
| 63  A9   | PB18
| 64  A10  | PB19
| 65  A11  | PB20
| 52       | PB21
| -- 3 bits skipped --
| 2        | PB25
| 22       | PB26
| 13       | PB27
| -- 5 bits skipped --
| 33       | PC1
| 34       | PC2
| 35       | PC3
| 36       | PC4
| 37       | PC5
| 38       | PC6
| 39       | PC7
| 40       | PC8
| 41       | PC9
| -- 2 bits skipped --
| 51       | PC12
| 50       | PC13
| 49       | PC14
| 48       | PC15
| 47       | PC16
| 46       | PC17
| 45       | PC18
| 44       | PC19
| -- 1 bit skipped --
| 9        | PC21
| 8        | PC22
| 7        | PC23
| 6        | PC24
| 5        | PC25
| -- 2 bits skipped --
| 3        | PC28
| -- 4 bits skipped
| 25       | PD0
| 26       | PD1
| 27       | PD2
| 28       | PD3
| 14       | PD4
| 15       | PD5
| 29       | PD6
| 11       | PD7
| 12       | PD8
| 30       | PD9
| 32       | PD10
| -- 22 bits skipped

## References

[Open Music Labs - Input Matrix Scanning Overview](http://www.openmusiclabs.com/learning/digital/input-matrix-scanning/index.html)
This is an older (2011) but good tutorial describing many ways that keyboard input can be read.  It includes some hardware based
methods that predate microprocessor based designs.

[Arduino Due Pinout (showing port mapping)](https://content.arduino.cc/assets/Pinout-Due_latest.pdf)

[Arduino Mega 2560 Port Mapping](https://docs.arduino.cc/retired/hacking/hardware/PinMapping2560/)
