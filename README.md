# grad-cap

> bths '21 graduation quad seven segment driver

This is what happens when you combine some playful creativity, boredom, and
over-the-top determination the day before graduation.


## Features

* Multiplexing (~160Hz refresh rate per digit)
* Animated scrolling messages
* Power saving optimizations


## Hardware Configuration

Hardware Components:

* Atmel ATmega328P
* 20MHz crystal oscillator
* quad seven segment display
* 330ohm resistor pack
* 5V power source

Hardware Connections:

```
               +--*--+
              -|     |-
  seg a - PD0 -|     |-
  seg b - PD1 -|  A  |-
  seg c - PD2 -|  T  |-
  seg d - PD3 -|  m  |-
  seg e - PD4 -|  e  |-
          VCC -|  g  |-
          GND -|  a  |-
      crystal -|  3  |-
      crystal -|  2  |-
  seg f - PD5 -|  8  |-
  seg g - PD6 -|  P  |- PB3 - dig 3
     dp - PD7 -|     |- PB2 - dig 2
  dig 0 - PB0 -|     |- PB1 - dig 1
               +-----+
```


## Design Decisions

Originally this project was intended to be a quick and dirty hack of the
[original quad seven segment driver] I wrote for class.  Unfortunately, while
doing some initial tests, I discovered the original code was horrendously
inefficient: 30m of runtime dropped a 9V battery’s voltage by ~0.4V.


### Configuration

To reduce SRAM usage, the program uses bit packing to store a single pattern
into one byte.  Given the structure of the ATmega328P, PORTD is the only
hardware register with access to eight digital I/O, a perfect match for the
eight segments of the display.  With this in mind, patterns can be directly
written from SRAM to the hardware register, vastly simplifying programming and
speeding up program execution.

Since the hardware requires multiplexing, the lower range of the PORTB hardware
register mapped to each digit enable pin.  With such a configuration, enabling
the appropriate digit became trivial, only requiring a bit-shift in memory.


### Optimizations

The original version of the driver I had written had no intention of being
power efficient.  In actuality, I had written it to be as fast as possible,
getting close to maxing out the hardware capabilities of the ATmega328P when
clocked at 20MHz.  To circumvent the shortcomings of the original design, the
new program utilizes several optimizations to reduce power draw.

In short, three optimizations were put in place: the refresh rate was lowered,
unnecessary hardware modules were disabled, and power-save mode was enabled
where possible.  When the program initializes, all unnecessary hardware modules
are disabled, and TIMER2 is set to trigger an overflow interrupt approximately
every 1.6ms.  Once in the main loop, the program updates all counters and sets
PORTD and PORTB to the appropriate state before going into power-save mode.
Finally, the program waits for the interrupt from TIMER2 to exit power-save
mode.  This loop then repeats indefinitely.

With these optimizations enabled, power usage decreased by ~6x as ~3h of usage
at graduation resulted in a ~0.4V drop in voltage in the 9V battery powering
the driver.


## Miscellaneous Information

* This program was compiled using the [AVR Toolchain] (avr-libc v2.0.0,
  avr-binutils v2.36.1, avr-gcc v11.1.0), and flashed using avrdude v6.3.0.
* PROGMEM could have been used for storing the messages, but it seemed
  unnecessary given how little messages I wanted to display.
* Size wasn't an issue so the program was compiled with `-02` optimizations.
* Check out the `bths-grad` tag to view the version of the program flashed to
  the driver during graduation.


## Copyright & Licensing

Copyright (C) 2021  Jacob Koziej <jacobkoziej@gmail.com>

Distributed under the [GPLv3].


[original quad seven segment driver]: https://github.com/bths-mechatronics-robotics-21/cookie-jar/blob/master/seven-seg-driver/seven-seg-driver.ino
[AVR Toolchain]: https://www.nongnu.org/avr-libc/
[GPLv3]: LICENSE.md
