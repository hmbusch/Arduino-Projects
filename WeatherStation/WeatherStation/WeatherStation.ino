#include <Arduino.h>
#include <U8g2lib.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_TSL2561_U.h>

#include <SPI.h>
#include <Wire.h>

#define DEBUG

#include "debug.h"

// Set up the Waveshare 1.3 OLED display (128x64 pixel, SH1106 controller, noname display)
U8G2_SH1106_128X64_NONAME_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 8, /* reset=*/ 5);

// Set up the BME280 temperature/humidtity/pressure sensor
Adafruit_BME280 bme;

// Set up the TSL2561 lux sensor
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

float sensorTemperature;
float sensorHumidity;
int sensorLux;
int sensorPressure;

int switchBrightnessPressureCounter = 0;
boolean drawBrightness = true;

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

  // Initialize the BME280 sensor
  if (!bme.begin(0x76)) {
    Serial.println("[ERROR] Couldn't find any BME280 device, check your I2C addresses or your cabling");
    while (1);
  }
  
  u8g2.begin();  
  u8g2.enableUTF8Print();
}

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

void readBME280() {
  // Read the BME280
  sensorPressure = bme.readPressure() / 100.0F;
  if (sensorPressure < 0) {
    sensorPressure = 0;
  }
  sensorTemperature = bme.readTemperature();
  sensorHumidity = bme.readHumidity();
}

void loop() {
  readBME280();
  readTSL2561();

  if (switchBrightnessPressureCounter == 20) {
    switchBrightnessPressureCounter = 0;
    drawBrightness = !drawBrightness;   
  }
  
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

    if (drawBrightness) {
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
    } else {
      // draw air pressure
      u8g2.setFont(u8g2_font_open_iconic_thing_2x_t);
      u8g2.drawGlyph(1, 64, 73);
      u8g2.setFont(u8g2_font_profont12_tf);
      u8g2.setCursor(22, 60);
      if (sensorPressure < 1000) {
        u8g2.print(" ");  
      }
      u8g2.print(sensorPressure);
      u8g2.print(" hPa");     
    }

    // draw min temp
    u8g2.setFont(u8g2_font_profont15_tf);
    u8g2.setCursor(84, 12);
    u8g2.print("Min");
    u8g2.setCursor(88, 28);
    u8g2.setFont(u8g2_font_profont12_tf);
    u8g2.print(sensorTemperature, 1);
    u8g2.print("°C");

    // draw min temp
    u8g2.setFont(u8g2_font_profont15_tf);
    u8g2.setCursor(84, 45);
    u8g2.print("Max");
    u8g2.setCursor(88, 61);
    u8g2.setFont(u8g2_font_profont12_tf);
    u8g2.print(sensorTemperature, 1);
    u8g2.print("°C");
    
  }
  while(u8g2.nextPage());
  switchBrightnessPressureCounter++;
  delay(1000);
}
