/**
 * Simple Weather Station
 * 
 * by Hendrik Busch, 2018
 */

#include <Arduino.h>
#include <U8g2lib.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <DHT.h>
#include <DHT_U.h>

#include <SPI.h>
#include <Wire.h>

// Uncomment this if you want debugging. Enabling debugging reduced the number
// of recorded historical values from 144 (= 12 hours) to 12 (= 1 hour),
// otherwise there would'nt be enough RAM to facilitate debugging and
// historical values at the same time.
//#define DEBUG

#ifdef DEBUG
  #define MAX_HISTORY 12
#else
  #define MAX_HISTORY 144
#endif

#include "debug.h"

/** 
 * Set up the Waveshare 1.3 OLED display access object (128x64 pixel, SH1106 controller, noname display) 
 */
U8G2_SH1106_128X64_NONAME_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 8, /* reset=*/ 5);

/**
 * Set up the TSL2561 lux sensor access object.
 */
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

/** 
 * Set up the DHT22 temperature & humidity sensor access object 
 */
DHT_Unified dht(2, DHT22);

/**
 * Field to store the latest temperature reading in.
 */
float sensorTemperature;

/**
 * Field to store the latest humidity reading in.
 */
float sensorHumidity;

/**
 * Field to store the latest brightness reading in.
 */
int sensorLux;

/**
 * The currently known maximum temperature.
 */
float currentMinTemperature;

/**
 * The currently know minimum temperature.
 */
float currentMaxTemperature;

/**
 * The time passed since the last historical value was recorded.
 */
unsigned long lastHistoryTime = 0;

/**
 * The index of the historical values that was last written to.
 */
int lastHistoryIndex = -1;

/**
 * The highest index that has been written to already.
 */
int maxHistoryIndex = -1;

/**
 * Store the last 144 temperature readings (12 hours when storing once every 5 minutes)
 */
float historicalTemperatures[MAX_HISTORY];

/**
 * If this flag is true, a recalculation of the minimum and maximum temperature is
 * required. The flag is set to true when new historical values have been added and
 * is set to false once the new calculation has completed.
 * It is initially set to true so that min/max temperatures are available for display
 * at any time.
 */
boolean minMaxUpdateRequired = true;

void setup() {
#ifdef DEBUG  
  Serial.begin(57600);
#endif

  // Configure the light sensor for auto-range and fast (but slightly inaccurate) readings
  if (tsl.begin()) {
    tsl.enableAutoRange(true);
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
  }
  else {
    DEBUG_PRINTLN("[ERROR] Couldn't find any TSL2561 device, check your I2C addresses or your cabling");
  }

  dht.begin();
  
  u8g2.begin();  
  u8g2.enableUTF8Print();
}

/**
 * Reads the current brightness level (in lux) from the sensor and puts
 * the current value into the field 'sensorLux'.
 */
void readTSL2561() {
  // Read the light sensor
  sensors_event_t lightEvent;
  tsl.getEvent(&lightEvent);
  if (lightEvent.light > 0) {
    sensorLux = lightEvent.light;     
  }
  else {
    sensorLux = 0;
  }
}

/**
 * Reads temperature and humidity using the DHT22 sensor. As the Adafruit Unified
 * Sensor Library is event based, two readings are neccessary to get both values.
 * The readings are applied to the 'sensorTemperature' and 'sensorHumidity' fields.
 */
void readDHT22() {
  sensors_event_t event;  

  // read temperature
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
     DEBUG_PRINTLN("Error reading temperature from DHT22");
     sensorTemperature = 0;
  } else {
    sensorTemperature = event.temperature;
  }

  // read humidity
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    DEBUG_PRINTLN("Error reading humidity from DHT22");
    sensorHumidity = 0;
  } else {
    sensorHumidity = event.relative_humidity;
  }
}

/**
 * Draws the min/max display segments.
 * 
 * @param name the heading to print
 * @param x the x-coordinate where to start printing
 * @param y the y-coordinate where to start printing
 * @param value the value to print
 */
void drawMinMax(const char *name, uint8_t x, uint8_t y, float value) {
    u8g2.setFont(u8g2_font_profont15_tf);
    u8g2.setCursor(x, y);
    u8g2.print(name);
    u8g2.setCursor(x + 4, y + 16);
    u8g2.setFont(u8g2_font_profont12_tf);
    u8g2.print(value, 1);
    u8g2.print("°C");
}

/**
 * Determines the minimum and maximum temperature by browsing through the 
 * historical values. This method updates the fields 'currentMinTemperature'
 * and 'currentMaxTemperature' with the corresponding values.
 * The calculation is only performed when the 'minMaxUpdateRequired' flag
 * indicates that there is new data available for consideration.
 */
void determineMinMaxTemperatureIfNeeded() {
  if (minMaxUpdateRequired) {
    DEBUG_PRINTLN("------------------------------------------------------------------------------------------");
    DEBUG_PRINTLN("Min/Max update required");
    currentMinTemperature = sensorTemperature;
    currentMaxTemperature = sensorTemperature;
    if (maxHistoryIndex >= 0) {
      DEBUG_PRINT("  Parsing ");
      DEBUG_PRINTDEC(maxHistoryIndex + 1);
      DEBUG_PRINTLN(" historical values");
      for(unsigned int i = 0; i <= maxHistoryIndex; i++) {
        currentMinTemperature = min(currentMinTemperature, historicalTemperatures[i]);      
        currentMaxTemperature = max(currentMaxTemperature, historicalTemperatures[i]);      

        DEBUG_PRINT("  ");
        DEBUG_PRINTDEC(i);
#ifdef DEBUG
        if (i < 10) {
          DEBUG_PRINT(" ");
        }
#endif        
        DEBUG_PRINT(": ");
        DEBUG_PRINTFLOAT(historicalTemperatures[i]);
        DEBUG_PRINT(" -> MIN: ");
        DEBUG_PRINTFLOAT(currentMinTemperature);
        DEBUG_PRINT(" / MAX: ");
        DEBUG_PRINTFLOAT(currentMaxTemperature);
        DEBUG_PRINTLN("");
      }
    } else {
      DEBUG_PRINTLN("  no historical values recorded yet");
    }
    minMaxUpdateRequired = false;
    DEBUG_PRINTLN("------------------------------------------------------------------------------------------");
  } else {
    currentMinTemperature = min(currentMinTemperature, sensorTemperature);  
    currentMaxTemperature = max(currentMaxTemperature, sensorTemperature);
  }
}

/**
 * This method is called in every loop iteration and records historical
 * temperature values if needed. It uses 'historicalTemperatures' as a 
 * ring buffer to record and store the last 12 hours of data, generating
 * one value every 5 minutes. Once a new value is added, the 'minMaxUpdateRequired'
 * flag is set to true so that new min/max temperatures are calculated.
 */
void recordHistoricalTemperatureIfNeeded() {
  if (millis() - lastHistoryTime > 300000 || lastHistoryTime == 0) {
    int newIndex = lastHistoryIndex + 1;
    if (newIndex >= MAX_HISTORY) {
      newIndex = 0;
    }
    maxHistoryIndex = max(maxHistoryIndex, newIndex);
    historicalTemperatures[newIndex] = sensorTemperature;   
    lastHistoryIndex = newIndex;
    lastHistoryTime = millis();
    minMaxUpdateRequired = true;
    
    DEBUG_PRINT("# Recording temperature ");
    DEBUG_PRINTFLOAT(sensorTemperature);
    DEBUG_PRINT(" at index ");
    DEBUG_PRINTDEC(newIndex);
    DEBUG_PRINT(", new max index is ");
    DEBUG_PRINTDEC(maxHistoryIndex);
    DEBUG_PRINT(", latest history index is ");
    DEBUG_PRINTDEC(lastHistoryIndex);
    DEBUG_PRINTLN("");
  }
}

void loop() {
  readTSL2561();
  readDHT22();

  recordHistoricalTemperatureIfNeeded();
  determineMinMaxTemperatureIfNeeded();

  u8g2.firstPage();
  do {
    // draw temperature
    u8g2.setFont(u8g2_font_profont22_tf);
    u8g2.setCursor(0, 22);
    u8g2.print(sensorTemperature, 1);
    u8g2.print("°C");
    
    // draw seperation Lines
    u8g2.drawVLine(78, 1, 63);
    u8g2.drawHLine(1, 27, 74);

    // draw humidity
    u8g2.setFont(u8g2_font_open_iconic_thing_2x_t);   
    u8g2.drawGlyph(1, 47, 72);
    u8g2.setFont(u8g2_font_profont12_tf);
    u8g2.setCursor(40, 43);
    u8g2.print(sensorHumidity, 0);  
    u8g2.print(" %");

    // draw brightness
    u8g2.setFont(u8g2_font_open_iconic_weather_2x_t);
    u8g2.drawGlyph(1, 64, 69);
    u8g2.setFont(u8g2_font_profont12_tf);
    u8g2.setCursor(22, 60);
    if (sensorLux < 1000) {
      u8g2.print(" ");  
    }
    if (sensorLux < 100) {
      u8g2.print(" ");  
    }
    u8g2.print(sensorLux);
    u8g2.print(" lux");  

    drawMinMax("Min", 84, 12, currentMinTemperature);
    drawMinMax("Max", 84, 45, currentMaxTemperature);
  }
  while(u8g2.nextPage());

  // delay next update for 2 seconds
  delay(2000);
}
