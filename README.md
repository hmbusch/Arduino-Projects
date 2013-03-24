----------
# Arduino-Projects #
URL: https://github.com/hmbusch/Arduino-Projects                   


----------

Whereas the Arduino-Experiments Repository only contains snippets and small examples of how to do certain things with the Arduino, this repository contains larger projects, often accompanied by some form of hardware design. When 'Experiments' is the toolbox, then this is the final product. Or sort of... whatever. Enjoy!

## Contents

### 1 - StepperDriver

The StepperDriver is just what it claims to be: a simple design for a stepper motor controller that uses a L297/L298 combination and can be built using a single sided pcb (ok, you have to add a lot of bridge wires, but it's still easier than etching a double sided pcb). The project currently consists only of the TARGET 3001! schematics, but it uses a fairly standardized interface with the same pins as most other stepper controllers.

The StepperDriver is based on the [RepRap Stepper Motor Driver 1.2](http://reprap.org/wiki/Stepper_Motor_Driver_1.2).

In fact, I only removed some components (like the min/max sensors)
and created a completely new board layout so that everything can
be home etched.

### 2 - Solar Meter

The solar meter is a voltage measurements circuit for monitoring my small solar power installation.

### 3 - LED-Matrix

This project features an DIY 8x8 LED matrix together with a controller board that uses a shift register and a decade counter. Sample code for scrolling text on the matrix is also provided. 

The code relies heavily on code orignally provided at:
[Wise Time With Arduino - Scrolling text on the Dual RG matrix shield](http://timewitharduino.blogspot.com/2010/03/scrolling-text-on-dual-rg-matrix-shield.html)

### 4 - Binary Clock

A binary clock using high power LEDs for display. It features a modular design and features a real time clock module. [**Not yet completed**]

### 5 - Breakouts

This is an experimental series of small breakout pcbs that provide a
simple way of connecting components to the Arduino and that provide
all the necessary circuitry, such as pull-up/down resistors.

### 6 - WiFiStrengthLEDBar

Inspired by the ["Immaterials: WiFi Light Painting"](http://vimeo.com/20412632) video, this project features a 40 LED bar for displaying the signal strength of a WiFi network measured with an Arduino WiFi Shield. [**Not yet completed**]

