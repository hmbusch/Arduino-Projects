/**
 * ICMI Solar Meter v1.2
 * 2013, Hendrik Busch (http://www.icanmakeit.de)
 * 
 * This project is a DIY meter originally intended for my solar energy
 * installation. It is supposed to add monitoring functionality to my
 * off-the-shelve solar charger, that lacks this functionality.
 *
 * This third iteration monitors the voltage battery and logs the measured
 * voltages to SD card for later evaluation. In a later stage, things like 
 * power intake and output will be measurable. Measuring the panel voltage
 * has been removed from this version because my charger has some issues 
 * when the battery and the solar panel share a common ground (GND) connection.
 * Until I have found a way around this, panel voltage will no longer be
 * monitored.
 *
 * The circuit consists of a voltage divider allowing the Arduino to
 * measures voltages of up to 25 Volts, connected to analog 0.
 * The divider is made up as follows: 
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
 * For time keeping and logging purposes, this circuit also uses the Adafruit Logger Shield
 * (http://www.ladyada.net/make/logshield/index.html) that occupies analog 4&5 (I2C) as well as
 * digital pins 10-13. In exchange, it provides a 3.3V power source that works better than the
 * integrated one on the Arduino.
 *
 * Although the DS1307 real-time clock is pretty reliable, it can't hurt to have extra precision.
 * This is why I added an DCF77 receiver to the circuit. This device runs on 3.3 Volts, the data
 * pin is connected to digital pin 2.
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
 
#include <LiquidCrystal.h>   // comes with Arduino
#include <Wire.h>            // comes with Arduino
#include "RTClib.h"          // https://github.com/adafruit/RTClib
#include <DCF77.h>           // https://github.com/thijse/Arduino-Libraries/downloads
#include <Time.h>            // http://www.arduino.cc/playground/Code/Time
#include <SD.h>              // https://github.com/adafruit/SD

//===================================
//   Voltage measurement settings
//===================================

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

//===================================
//         time settings
//===================================

// the DFC77 data pin
#define DCF_PIN 2	

// the interrupt that is associated with the data pin
#define DCF_INTERRUPT 0

// type declaration for the time struct
time_t time;

// Initialise the receiver with pin and interrupt settings
DCF77 DCF = DCF77(DCF_PIN,DCF_INTERRUPT);

// Construct the RTC handler
RTC_DS1307 RTC;

//===================================
//         Display settings
//===================================

// initialize the LCD with the appropriate pins
LiquidCrystal lcd(8, 7, 6, 5, 4, 3);

// Custom glyph for a backslash, some displays don't
// display it properly.
byte backslash[8] = {
        B00000,
        B10000,
        B01000,
        B00100,
        B00010,
        B00001,
        B00000,
        B00000
};


//===================================
//         SD card settings
//===================================

// the pin that is used for SD card chip select (pin 10 on the Adafruit SD shield)
#define SD_CHIP_SELECT 10

// SD card handler
Sd2Card card;

// SD volume handler
SdVolume volume;

// The file handle for the log file
File logfile;

//===================================
//        runtime variables
//===================================

/*
 * When this indicator is true, the system will try to get the 
 * current time from the DCF receiver and update the RTC.
 */
boolean needsDCFRefresh = true;

/*
 * The loop count since the last RTC update. It makes no sense
 * updating the RTC in every iteration and thus we wait.
 * This variable also doubles as a frame indicator for the waiting
 * animation (when used with the modulo operator).
 */
int cyclesSinceLastDCFRefresh = 0;

/*
 * These are the symbols needed to display a small animation.
 */
char animation[] = {'/','-','\\','-'};

/*
 *
 */
long lastLogTime;

/**
 * Initializes all the required objects and 
 * displays a welcome message;
 */
void setup()
{
   Wire.begin();
   RTC.begin();
   DCF.Start();
   Serial.begin(9600);
   
  // Switch the ADC reference voltage to 1.1 Volts
  analogReference(INTERNAL);

  lcd.createChar(0, backslash);
  lcd.begin(16, 2);
  lcd.print("ICMI Solar Meter");  
  lcd.setCursor(0,1);
  lcd.print("      v1.2");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("-> github.com/  ");
  lcd.setCursor(0,1);
  lcd.print("    hmbusch");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  
  // chip select pin (pin 10 on Arduino Uno) must be an output, otherwise SD won't work
  pinMode(SD_CHIP_SELECT, OUTPUT);
  
  // Perform low level initialization of SD for checks and volume size
  /*
  if (!card.init(SPI_FULL_SPEED, SD_CHIP_SELECT))
  {
    lcd.print("SD card error!");
    lcd.setCursor(0,1);
    lcd.print("Check card!");
    delay(5 * 60 * 1000); 
  }
  else
  {
    if (volume.init(card))
    {
      lcd.print("SD card OK");
      lcd.setCursor(0,1);
      long volumeSize = volume.blocksPerCluster() * volume.clusterCount() * 512;
      lcd.print(volumeSize / 1048576, DEC);
      lcd.print(" MB");
    }
    else
    {
      lcd.print("Filesystem error");
      lcd.setCursor(0,1);
      lcd.print("Please reformat");
    }
  }
  
  delay(2000);
  lcd.clear();
  */
  
  // Now perform high level initialization for hassle free file access;
  if (SD.begin(SD_CHIP_SELECT))
  {
    lcd.print("SD card OK");
  }
  else
  {
    lcd.print("SD card error!");  
    while(true);
  }
  
  // create a new file with a unique name
  char filename[] = "log_00.csv";
  for (int i = 0; i < 100; i++) 
  {
    filename[4] = i/10 + '0';
    filename[5] = i%10 + '0';
    if (!SD.exists(filename)) 
    {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      lcd.setCursor(0,1);
      if (logfile)
      {
        lcd.print(filename);
        logfile.println("\"time\",\"voltage\",\"raw analog value\"");
        logfile.flush();
        break;  // leave the loop!
      }
      else
      {
        lcd.print("File error!");
        while(true);
      }
    }
  }
  
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
  handleTime();
  
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
 lcd.setCursor(0,1);
 
 // This prints out a nice floating point formatted values with 2 digits behind
 // the separator.
 lcd.print("Vbat: ");
 lcd.print(vBattery/1000.0, 2);
 lcd.print(" V    ");
 
 // log the value (method decides whether to log or to wait)
 logVoltage(vBattery, rawAverageVBattery);
 
 // wait a bit before next iteration
 delay(1000);
} 
 
void logVoltage(int milliVolts, int rawValue)
{
  DateTime now = RTC.now();
  if (now.unixtime() - lastLogTime > 30)
  {
    Serial.print("Last log time: ");
    Serial.println(lastLogTime, DEC);
    Serial.print("Now          : ");
    Serial.println(now.unixtime(), DEC);
    Serial.print("Difference   : ");
    Serial.println(now.unixtime() - lastLogTime, DEC);
    
    // outut csv data
    logfile.print('"');
    logfile.print(now.year(), DEC);    
    logfile.print('-');
    logfile.print(now.month() < 10 ? "-0" : "-");
    logfile.print(now.month(), DEC);
    logfile.print(now.day() < 10 ? "-0" : "-");
    logfile.print(now.day(), DEC);
    logfile.print(' ');
    if (now.hour() < 10) logfile.print('0');
    logfile.print(now.hour(), DEC);
    logfile.print(now.minute() < 10 ? ":0" : ":");
    logfile.print(now.minute(), DEC);
    logfile.print(now.second() < 10 ? ":0" : ":");
    logfile.print(now.second(), DEC);
    logfile.print("\",");
    logfile.print(milliVolts, DEC);
    logfile.print(",");
    logfile.print(rawValue, DEC);
    logfile.println();
    
    // signal a write operation and flush the new line to the SD card
    lcd.setCursor(15,1);
    lcd.print("!");
    logfile.flush();
    lcd.setCursor(15,1);
    lcd.print(" ");
    
    lastLogTime = now.unixtime();
  }
}
 
  
/** 
 * Performs all the neccessary operations to sync DCF and RTC
 * and display the current time on the LCD.
 */
void handleTime()
{
  // Erase any animation marker
  lcd.setCursor(15,0);
  lcd.print(' ');
  
  if(RTC.isrunning())
  {
    // a DCF refresh is preferrable
    if (needsDCFRefresh)
    {
      performDCFRefresh();
    }
    else
    { 
      cyclesSinceLastDCFRefresh++;
      if (cyclesSinceLastDCFRefresh > 3600)
      {
        needsDCFRefresh = true;
        cyclesSinceLastDCFRefresh = 0;
      }
    }
    displayTime();
  }
  else
  {
    lcd.setCursor(0,0);
    lcd.print("[no time]");
    performDCFRefresh();
  }
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

/**
 * Tries to read the current time from the DCF receiver and applies it
 * the the RTC. When the DCF time is not available, this method updates
 * a small animation on the display.
 */
void performDCFRefresh()
{
  // get the DCF time and apply it to the RTC when available
  time_t DCFtime = DCF.getTime();
  if (DCFtime != 0)
  {
    tmElements_t tm;   
    breakTime(DCFtime, tm);
    RTC.adjust(DateTime(tm.Year+1970, tm.Month, tm.Day,tm.Hour, tm.Minute, tm.Second));
    needsDCFRefresh = false;
    cyclesSinceLastDCFRefresh = 0;
    lcd.setCursor(0,0);
    lcd.print("[DCF77 set ok]");
    Serial.println("RTC updated with DCF77 data");
  }
  else
  {
    lcd.setCursor(15,0);
    int frameNumber = cyclesSinceLastDCFRefresh % 4;
    // Stupid workaround for displaying a backslash properly.
    if (frameNumber == 2)
    {
      lcd.write(byte(0));
    }
    else
    {
      lcd.print(animation[frameNumber]);
    }
    cyclesSinceLastDCFRefresh++;
  }
}

/**
 * Reads the time from the RTC clock and displays it on the
 * LCD starting in row 0/column 0. The format is yyyy-MM-dd HH:mm:ss.
 */
void displayTime()
{
  lcd.setCursor(0,0);
  DateTime now = RTC.now();
  printDigits(now.year()-2000);
  lcd.print('-');
  printDigits(now.month());
  lcd.print('-');
  printDigits(now.day());
  lcd.print(' ');
  printDigits(now.hour());
  lcd.print(':');
  printDigits(now.minute());
}

/**
 * Print the given number to the LCD using decimal encoding.
 * When the number is lower than 10, a leading zero is added
 * to pad the output.
 */
void printDigits(int digits)
{
  if(digits < 10)
  {
    lcd.print('0');
  }
  lcd.print(digits, DEC);
}
