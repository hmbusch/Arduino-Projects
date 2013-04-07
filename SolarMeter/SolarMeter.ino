/**
 * ICMI Solar Meter v1.1
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
 * measures voltages of up to 25 Volts, connected to analog 0 and analog 1.
 * The dividers are made up as follows: 
 *     
 *    V+ -> 68K resistor -> analog pin -> 5K potentiometer (set to 3K) -> GND
 *
 * I opted for a potentiometer to allow fine tuning without changing the software. 
 * Also, the reference voltage for the ADC is no longer 5 Volts but 1.1 Volts. This
 * is a little more precise because the main problem are rounding errors and overflows
 * when calculating the real voltage. To avoid floating point math for as long as possible,
 * voltage is calculated in millivolts and a maximum of 1100 millivolts leave a 
 * little more room than 5000.
 *
 * The LCD uses the following connections:
 *
 * LCD RS pin to digital pin 8
 * LCD Enable pin to digital pin 7
 * LCD D4 pin to digital pin 6
 * LCD D5 pin to digital pin 5
 * LCD D6 pin to digital pin 4
 * LCD D7 pin to digital pin 3
 * LCD R/W pin to ground
 * a potentiometer on V0 for controlling display contrast.
 *
 * WARNING: This circuit can only withstand 25 Volts *at maximum*.
 * - Do not apply a voltage greater than that! 
 * - Check you polarity when making connections!
 * - Make sure you set the potentiometer to  about 3K resistance before connecting anything!
 *
 * Using a voltage outside of 0-25 Volts and/or mixing up polarity will
 * almost certainly destroy your Arduino and/or worse. I can and will 
 * take absolutely no responsiblity for things that may go wrong when you 
 * build this project.
 *
 * NOTE: Fine tune your potentiometer using a multimeter and a realistic battery voltage.
 * There usually is deviation between the real voltage and the one calculated. This deviation
 * grows the farther the current voltage is apart from the voltage the circuit was fine-tuned for.
 */
 
#include <LiquidCrystal.h> 

// Battery is connected to the voltage divider at analog 0
#define SENS_BATTERY 0

// the internal ADC reference voltage (1.1 Volts) in millivolts;
#define REF_VOLTAGE 1100

// The factor by which to multiply the measured voltage in order
// to calculate the real voltage. The real factor between 68K and 3K
// is 23.81, but I try to avoid floating point math as long as possible
// and will later divide the final result by 100.
int vInVOutFactor = 2381;

// the number of readings to take for building an average
int numberOfReadings = 5;

// initialize the LCD with the appropriate pins
LiquidCrystal lcd(8, 7, 6, 5, 4, 3);

/**
 * Prints a small welcome message and nothing more.
 */
void setup()
{
  analogReference(INTERNAL);
  lcd.begin(16, 2);
  lcd.print("ICMI Solar Meter");  
  lcd.setCursor(0,1);
  lcd.print("      v1.1");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("-> github.com/  ");
  lcd.setCursor(0,1);
  lcd.print("    hmbusch");
  delay(2000);
  lcd.clear();
}

/**
 * Measures and displays the voltage of the solar panel and the battery
 * by reading the values, averaging them, calculating the real voltage
 * and updating the display.
 * The delay between two measurements is 200ms. Building the average avoids
 * jumpy readings.
 */
void loop()
{
  int rawVBattery[numberOfReadings];
  
  // Take a number readings with 200ms delay and build an average
  for (int i = 0; i < numberOfReadings; i++)
  {
    rawVBattery[i] = analogRead(SENS_BATTERY);
    delay(200);
  }
 
 int rawAverageVBattery = averageValue(rawVBattery, numberOfReadings);

 // Map the measured value to a range of 0-1100 millivolts and scale them up 
 // using the voltage divider ratio and the correction factor. The result is
 // the real voltage in millivolts (i.e. in a range from 0-25000).
 int vBattery = map(rawAverageVBattery, 0, 1023, 0, REF_VOLTAGE) * vInVOutFactor / 100;
 
 // Print the results
 lcd.clear();
 lcd.setCursor(0,0);
 
 // This prints out a nice floating point formatted values with 2 digits behind
 // the separator.
 lcd.print("Battery: ");
 lcd.print(vBattery/1000.0, 2);
 lcd.print(" V    ");
 delay(1000);
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
