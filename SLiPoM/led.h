#ifndef LED_H
#define LED_H

/**
 * Inits the NeoPixel and the timer used to blink it.
 */
void initLED();

/**
 * Makes the timer blink {@code repeatCount} times with an interval of {@code period}
 * between on-off or off-on. The pixel will blink in the given color.
 * 
 * @param period the interval between an on-off or off-on change, in milliseconds
 * @param repeatCount the number of times the pixel should blink
 * @param colorP the color the pixel should blink in. Use {@code makeColor()} to construct
 *               a suitable value for this parameter.
 */
void blinkPixel(uint32_t period, uint8_t repeatCount, uint32_t colorP);

/**
 * Lights the pixel in the given color. If the color is black (0,0,0), the pixel will turn
 * off. This method will cancel any ongoing blinking. The pixel will remain lit until this 
 * method is called again or a new blinking begins.
 * 
 * @param colorP the color the pixel should light up in. Use {@code makeColor()} to construct
 *               a suitable value for this parameter.
 */
void lightPixel(uint32_t colorP);

/**
 * Creates a single value color from the given RGB values.
 * 
 * @param red the value for red, ranging from 0-255
 * @param green the value for green, ranging from 0-255
 * @param blue the value for blue, ranging from 0-255
 * 
 * @return a combined value that represents the configured color
 */
uint32_t makeColor(uint8_t red, uint8_t green, uint8_t blue);

#endif
