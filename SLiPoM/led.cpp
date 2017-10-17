#include <TimerOne.h>
#include <Adafruit_NeoPixel.h>
#include "led.h"
#include "settings.h"
#include "debug.h"

/**
 * The LED pixel used to signal the various states.
 */
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

/**
 * The object that controls the blinking patterns.
 */
TimerOne timer;

/**
 * The number of the current cycle the blinking process is in.
 */
volatile int currentCycle = 0;

/**
 * The number of cycles the pixel will need to complete. This is always twice the amount
 * of actual blinks (i.e. on time), e.g. on-off-on-off is two blinks but four cycles.
 */
volatile int cycleCount = 0;

/**
 * The color the pixel should blink in in it's current cycle.
 */
volatile uint32_t color = 0;

/**
 * Signals if the timer and thus the blinking is currently running.
 */
volatile boolean timerRunning = false;

/**
 * Inits the NeoPixel and the timer used to blink it.
 */
void initLED() {
  pixel.begin();
  pixel.setPixelColor(0, 0);
  pixel.show();
  timer.initialize(500 * 1000);
  timer.stop();
}

/**
 * Stops the timer and removed the callback function from the timers
 * interrupt so that it will not trigger anymore.
 */
void stopTimer() {
    timer.stop();
    timer.detachInterrupt();
    timerRunning = false;
}

/**
 * This function is used as a callback function every time the timer
 * triggers. It checks the cycle count to see if the pixel should be 
 * turned on or off or if the blinking has been completed.
 * Once the required number of cycles is reached, this function stops
 * the timer and detaches itself from the interrupt.
 */
void blinkCallback() {
  DEBUG_PRINT("blinkCallback() - This is cycle ");
  DEBUG_PRINTDEC(currentCycle);
  DEBUG_PRINT(" of ");
  DEBUG_PRINTDEC(cycleCount);
  DEBUG_PRINTLN("");

  if (currentCycle < cycleCount) {
    if (currentCycle == 0 || currentCycle % 2 == 0) {
      pixel.setPixelColor(0, color);
      DEBUG_PRINTLN("blinkCallback() - Turning pixel on");
    }
    else {
      pixel.setPixelColor(0, 0, 0, 0);
      DEBUG_PRINTLN("blinkCallback() - Turning pixel off");
    }
    pixel.show();
    currentCycle += 1;
  } else {
    DEBUG_PRINTLN("blinkCallback() - Reached cycle limit, stopping timer");
    stopTimer();
  }
}

/**
 * Makes the timer blink {@code repeatCount} times with an interval of {@code period}
 * between on-off or off-on. The pixel will blink in the given color.
 * 
 * @param period the interval between an on-off or off-on change, in milliseconds
 * @param repeatCount the number of times the pixel should blink
 * @param colorP the color the pixel should blink in. Use {@code makeColor()} to construct
 *               a suitable value for this parameter.
 */
void blinkPixel(uint32_t period, uint8_t repeatCount, uint32_t colorP) {
  currentCycle = 0;
  cycleCount = repeatCount * 2;
  color = colorP;
  DEBUG_PRINT("blinkPixel() - Setting ");
  DEBUG_PRINT(cycleCount);
  DEBUG_PRINT(" cycles with ");
  DEBUG_PRINT(period * 1000);
  DEBUG_PRINTLN(" microsecond intervals");
  timer.setPeriod(period * 1000);
  timer.attachInterrupt(&blinkCallback);
  timer.start();
  timerRunning = true;
}

/**
 * Lights the pixel in the given color. If the color is black (0,0,0), the pixel will turn
 * off. This method will cancel any ongoing blinking. The pixel will remain lit until this 
 * method is called again or a new blinking begins.
 * 
 * @param colorP the color the pixel should light up in. Use {@code makeColor()} to construct
 *               a suitable value for this parameter.
 */
void lightPixel(uint32_t colorP) {
  if (timerRunning) {
    DEBUG_PRINTLN("lightPixel() - Timer is currently running, stopping it so we can light the pixel");
    stopTimer();
  }
  pixel.setPixelColor(0, colorP);
  pixel.show();
}

/**
 * Creates a single value color from the given RGB values.
 * 
 * @param red the value for red, ranging from 0-255
 * @param green the value for green, ranging from 0-255
 * @param blue the value for blue, ranging from 0-255
 * 
 * @return a combined value that represents the configured color
 */
uint32_t makeColor(uint8_t red, uint8_t green, uint8_t blue) {
  return pixel.Color(red, green, blue);
}

