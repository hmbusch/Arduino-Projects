#ifndef SETTINGS_H
#define SETTINGS_H

/**
 * The debug flag. Remove the comment if you want the sketch
 * to display debugging information.
 */
//#define DEBUG

/**
 * The pin the NeoPixel is attached to.
 */
const int PIN_NEOPIXEL = 5;

/**
 * The analog pin from which the voltage is read.
 */
const int PIN_VOLTAGE = 3;

/**
 * The digital pin the door switch is attached to. Currently,
 * only closing switches with a pull-down resistor are supported.
 */
const int PIN_DOOR = 7;

/**
 * The lowest acceptable voltage per cell. If a battery
 * goes below this value times the cell count, the alarm
 * is raised.
 */
const int MIN_CELL_VOLTAGE = 3000;

/**
 * The voltage per cell at which the battery starts issuing 
 * warnings.
 */
const int LOW_CELL_VOLTAGE = 3300;


/**
 * The ratio by which to multiply the voltage read
 * from the voltage divider.
 */
const double VOLTAGE_RATIO = 1.0 / 0.357;

#endif
