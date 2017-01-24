# LED-Matrix README

## Links

- Source code & schematic: [Download it from GitHub](https://github.com/hmbusch/Arduino-Projects/tree/master/LED-Matrix)
- Video: [See it on YouTube](http://www.youtube.com/watch?v=g0VkgF8Dbhs)

## Version history

- 2011/03/24 - Intitial release
- 2017/01/24 - Updated the sketch
  - Compatible with the latest Arduino version 1.8.1
  - removed all proprietary hardware timer stuff in favor of the [TimerOne library](https://www.pjrc.com/teensy/td_libs_TimerOne.html)

## Requirements

AKA _you need these otherwise it won't work_.

- Arduino IDE (tested with 1.8.1)
- [TimerOne library](https://www.pjrc.com/teensy/td_libs_TimerOne.html) (install via Library Manager)
- Atmel compatible board (TimerOne won't run on every platform) 

##Overview

This project features an 8x8 LED matrix and a controller board for
use with e.g. the Arduino.

The controller connects to the 8 cathode rows and the 8 anode columns 
(or vice versa, orientation doesn't really matter) of the LED matrix
and provides a 5-pin control interface to use with any microcontroller.

It works by programming one row at a time using a serial-to-parallel
shift register (74HC595) and switching to the next row afterwards by
incrementing a decade counter (M74HC4017) that pulls the selected row
to ground using a MOSFET transistor (2N7000). By repeating this pattern
over and over and very fast, the POV (persistence of vision) effect
kicks in and the displayed image looks stable (with the eye unable to
register each single row).

The board is designed for a 5V power supply. Anything above that might
fry your ICs or LED, anything below that and the LEDs get dimmer 
and the ICs might not work as supposed.

## Files provided

In the `schematics` folder, you will find the schematics needed to 
fabricate both the LED matrix and the controller board. You can 
either use the TIF files and print them directly, setting the 
resolution to 600 dpi, or you can use the schematic files for 
TARGET 3001!, a circuit and PCD CAD program similar to EAGLE and
also available in a free version. Please refer to the [TARGET 3001! download page](http://server.ibfriedrich.com/wiki/ibfwikien/index.php/Download).

Pleas note that I designed this back in 2011 when I was relatively
new to electronics and really didn't know any better than to use
TARGET 3001!. With all the programs available today (2017), I would
have opted for EAGLE or KiCad instead.

The `code` folder contains a sample sketch to display some text
slowly scrolling over the matrix. It is quite well commented and
has been tested and verified, as long as you hook up your Arduino
correctly.

Please note that the sketch uses a timer library that may not be
compatible with all boards that can be programmed using the Arduino
IDE. It is intended to run with the AVR/Atmel 8-bit processors. It will
for instance not run on an ESP8266.

## Connections

The sketch uses five pins to control the display (apart from VCC and GND).
You need to hook the up as follows (provided you didn't change the pin
assignment in the sketch):

- shift register data : pin 11
- shift register clock: pin 12
- shift register latch: pin 13
- decade clock        : pin  7
- decade reset        : pin  6

# How to draw things using the matrix

Assuming you want to draw a single 8x8 frame, you first encode the frame
as a set of numbers from 0 two 255, each number a representation of a 
single row. For better understanding: try imagining each number in binary
form. Each 1 represents a lit LED, each 0 represents a dark LED. So if 
you'd want to light every other LED in a row, you would write:

``` 
B10101010 
```

which equals 170 in decimal form. A checkerboard image could therefor
be either written as:

```
B10101010
B01010101
B10101010
B01010101
B10101010
B01010101
B10101010
B01010101
```

or with regards to the program as:

```c_cpp
byte checkerboard[] = {170, 85, 170, 85, 170, 85, 170, 85};
```

To draw this frame, you need to give the controller the following 
sequence of commands (see also explanation below) and repeat them rapidly
(I'd suggest at least 60 times a second = 60 Hz. In the sketch provided
the redrawing is synchronized by using one of the ATmega's internal
timers):

```
01 for each row in the array
02     pull the register's latch low
03     for each bit in the current row
04         set the register's data line to the bit's value (HIGH or LOW)
05         pulse the register's clock pin
06     endfor
07     pull the register's latch high
08     pulse the decade's clock pin 
09     if the current row is the last one
10         pulse the decade's reset pin
11     endif
12 endfor
```
### Explanation

- Row 02: pulling the register's latch low is necessary because it prevents
        your changes from being displayed immediately. You would want to
        'load up' the register with all new bits before displaying them by
        pulling the latch high again.
		
- Rows 03-06: You could programm this manually, but there is a more efficient
            way in the Arduino library: shiftOut(). shiftOut() required the 
            data and the clock pin of the register, the byte order and the
            byte to 'shift out'. The byte order determines wether the 
            leftmost or the rightmost bit is the first to be displayed. 
            If your output appears upside down or mirrored, change the byte
            order here.
			
- Row 08: In reality, you would need to display a blank line (value 0) after 
        each row before pulsing the decade clock. When rapidly switching between
        the lines without clearing them befor stepping to the next, blurry
        'ghost lines' will appear.
		
- Rows 09-11: The decade has (as the name implies) 10 steps, but we only have 8
            rows. So we need to skip the last two steps on the decade by 
            resetting the decade to step 1 again.
			
That more or less it for hardware control. The rest is more or less 'just'
software coding.

Enjoy!
