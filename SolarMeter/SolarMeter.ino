/**
 * ICMI Solar Meter v1.0
 * 2013, Hendrik Busch (http://www.icanmakeit.de)
 * 
 * This project is a DIY meter originally intended for my solar energy
 * installation. It is supposed to add monitoring functionality to my
 * off-the-shelve solar charger, that lacks this functionality (or at least,
 * it has no display to report any values).
 *
 * This first version monitors the voltage of the solar cell panel and
 * the battery. In a later stage, things like power intake and output will
 * be measurable and I seek to implement logging features as well.
 *
 * The circuit consists of two voltage dividers allowing the Arduino to
 * measures voltages of up to 20 Volts, connected to analog 0 and analog 1.
 * The dividers are made up as follows: 
 *     
 *    V+ -> 150K resistor -> analog pin -> 50 K resistor -> GND
 *
 * Using these values, the measured voltage is exactly (at least mathematically)
 * 0.25 times the real voltage. This means that the real voltage can be calculated
 * by multiplying the measured voltage by 4.
 *
 * The LCD uses the following connections:
 *
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * a potentiometer on V0 for controlling display contrast.
 *
 * WARNING: This circuit can only withstand 20 Volts *at maximum*.
 * Do not apply a voltage greater than that. Check you polarity
 * when making connections. 
 * Using a voltage outside of 0-20 Volts and/or mixing up polarity will
 * almost certainly destroy your Arduino and/or worse. I can and will 
 * take absolutely no responsiblity for things that may go wrong when you 
 * build this project.
 */
 
#include <LiquidCrystal.h> 

// Solar panel is connected to the voltage divider at analog 0
#define SENS_PANEL 0

// Battery is connected to the voltage divider at analog 0
#define SENS_BATTERY 1

// The factor by which to multiply the measured voltage in order
// to calculate the real voltage.
int vInVOutFactor = 4;

// These "tuning" variables allow you to modify the vInVOutFactor for
// the panel and the battery individually. Modify those values until the
// Arduino shows approximately the same voltage as your multimeter does.
// Any value set here will be added to the vInVOutFactor, so to achieve
// lower readings, you must set these to negative.
float tuneFactorPanel = 0.045;
float tuneFactorBattery = 0.045;

// initialize the LCD with the appropriate pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

/**
 * Prints a small welcome message and nothing more.
 */
void setup()
{
  lcd.begin(16, 2);
  lcd.print("ICMI Solar Meter");  
  lcd.setCursor(0,1);
  lcd.print("      v1.0");
  delay(1000);
  lcd.clear();
}

/**
 * Measures and displays the voltage of the solar panel and the battery
 * by reading the values, averaging them, calculating the real voltage
 * and updating the display.
 * Measuring the three values for averaging takes 1.5 seconds. It is intended
 * for smoothing out jumpy values.
 */
void loop()
{
  int rawVPanel[3];
  int rawVBattery[3];
  
  // Take 3 readings with 500ms delay and build an average
  for (int i = 0; i < 3; i++)
  {
    rawVPanel[i] = analogRead(SENS_PANEL);
    rawVBattery[i] = analogRead(SENS_BATTERY);
    delay(500);
  }
 
 int rawAverageVPanel = averageValue(rawVPanel, 3); 
 int rawAverageVBattery = averageValue(rawVBattery, 3);

 // Map the measured value to a range of 0-5000 millivolts and scale them up 
 // using the voltage divider ratio and the correction factor. The result is
 // the real voltage in millivolts (i.e. in a range from 0-20000).
 int vPanel = map(rawAverageVPanel, 0, 1023, 0, 5000) * (vInVOutFactor + tuneFactorPanel);
 int vBattery = map(rawAverageVBattery, 0, 1023, 0, 5000) * (vInVOutFactor + tuneFactorBattery);
 
 // Print the results
 lcd.setCursor(0,0);
 lcd.print("Panel: ");
 
 // This prints out a nice floating point formatted values with 2 digits behind
 // the separator.
 lcd.print(vPanel/1000.0, 2);
 lcd.print(" V    ");
 lcd.setCursor(0,1);
 lcd.print("Battery: ");
 lcd.print(vBattery/1000.0, 2);
 lcd.print(" V    ");
} 
  
/**
 * Calculates the average of the values given in the array.
 * You also need to specify the size of the array for this
 * method to work.
 *
 * @param values the array of the values to build an average from
 * @param size   the size of the given array. If a size is given that
 *               is actually greater than the size of the array,
 *               this method will have unpredicable results.
 * @raturn       the calculated average
 */
int averageValue(int values[], int size)
{
  int sum = 0;
  for (int i = 0; i < size; i++)
  {
    sum += values[i];
  }
  return sum/size;
}
