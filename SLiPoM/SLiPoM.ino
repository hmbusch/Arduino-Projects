/**
 * Simple LiPo Monitor
 * 
 * Hendrik Busch, 2017, https://github.com/hmbusch
 * 
 * This sketch is intended to monitor the voltage of a 2S or 3S
 * LiPo battery with its only output being a single Neopixel LED.
 * 
 * I made this for my modified Nerf foam dart blaster. The circuit 
 * uses an Arduino Nano (Digispark might also work, but will probably
 * need a differend pin assignment) with a voltage divider to measure
 * the battery voltage. When activated, it flashes the cell count in
 * blue, then full volts in cyan and per remaining 100mV in purple. Finally,
 * an indication of the battery charge is given in either green, yellow
 * or red. If the door switch (corresponding to the jam door on my blaster)
 * is closed, the LED will then turn off and only reactivate if the
 * battery level drops. It will then flash either yellow or red or.
 * 
 * With the jam door open, the battery quality indicator will be on the
 * whole time.
 * 
 * If the LED flashes red repeatedly, the battery has fallen below the
 * lowest acceptable voltage and must not be used any further to 
 * prevent permanent damage.
 * 
 * Low voltage warning is triggered at 3.3V per cell, critical warning
 * is triggered at 3V per cell.
 */

#include "settings.h"
#include "debug.h"
#include "led.h"
#include "door.h"

/**
 * An enumeration with possible battery 'layouts' the sketch will
 * recognize during initialization. These are:
 * 
 * - CC_V_TOO_LOW  - The measured voltage is even below the minimum threshold
 *                   for the smallest battery size (i.e. below 6V)
 * - CC_V_TOO_HIGH - The measured voltage is too high, even for a 3S battery.
 *                   The battery should be disconnected as its voltage may damage
 *                   the circuit and or the blaster.
 * - CC_2_CELLS    - The battery is a 2S LiPo
 * - CC_3_CELLS    - The battery is a 3S LiPo
 */
enum cellcount {
  CC_V_TOO_LOW = 0,
  CC_V_TOO_HIGH,
  CC_2_CELLS,
  CC_3_CELLS
};

/**
 * An enumeration with all the states of the state machine that is used 
 * to control the command flow in the sketch.
 */
enum states {
  STATE_CELLCOUNT = 0,
  STATE_CHECK_VOLTAGE,
  STATE_BATTERY_LOW,
  STATE_BLINK_VOLTAGE,
  STATE_CHECK_DOOR
};

/**
 * The cell count determined in the initilization state.
 */
int cellCount = -3;

/**
 * The voltage threshold under which the voltage is considered low.
 * It is calculated by multiplying the determined cell count with
 * the value of {@code LOW_CELL_VOLTAGE} defined in the settings.h.
 * When the voltage descends through this threshold, the sketch will
 * issue warnings.
 */
int lowVoltageThreshold = 0;

/**
 * The voltage threshold under which the voltage is considered minimal.
 * It is calculated by multiplying the determined cell count with
 * the value of {@code MIN_CELL_VOLTAGE} defined in the settings.h.
 * When the voltage descends through this threshold, the sketch will
 * issue a permanent warning.
 */
int minVoltageThreshold = 0;

/**
 * The current state of the state machine.
 */
int currentState = STATE_CELLCOUNT;

/**
 * Flag if the jam door (or whatever switch you have hooked up)
 * was open during the last check.
 */
boolean doorCurrentlyOpen = false;

/**
 * Flag if the jam door (or whatever switch you have hooked up)
 * was open during the check before the last check.
 */
boolean doorPreviouslyOpen = false;

/**
 * Initializes the sketch by initializing Serial (if debug is
 * enabled), the LED and the door switch debouncing.
 */
void setup() {
#ifdef DEBUG  
  Serial.begin(57600);
#endif
  initLED();
  initDoor();
}

/**
 * Determines the battery cell count.
 * 
 * @return one of the battery states, they are described at the top
 *         of the sketch
 */
int determineCellCount() {
  int batteryVoltage = determineBatteryVoltage();
  if (batteryVoltage < 6000) {
    return CC_V_TOO_LOW;
  }
  if (batteryVoltage < 8500) {
    return CC_2_CELLS;
  }
  if (batteryVoltage > 12800) {
    return CC_V_TOO_HIGH;
  }
  return CC_3_CELLS;
}

/**
 * Measures the battery voltage on the configured analog pin
 * and returns it as millivolts.
 *
 * @return the battery voltage in millivolts
 */
int determineBatteryVoltage() {
  int analogValue = readAnalog(PIN_VOLTAGE);
  // 4992 is the theoretical maximum voltage (in mV) when using
  // 14 Volts as maximum input for the divider using the given
  // resistor values.
  int milliVolts = map(analogValue, 0, 1023, 0, 4992);
  int milliVoltsBattery = VOLTAGE_RATIO * milliVolts;
  DEBUG_PRINT("determineBatteryVoltage() - ");
  DEBUG_PRINT(analogValue);
  DEBUG_PRINT(" / ");
  DEBUG_PRINT(milliVolts);
  DEBUG_PRINT(" / ");
  DEBUG_PRINT(milliVoltsBattery);
  DEBUG_PRINTLN(" (analog value / mV at divider / mV Battery)");
  return milliVoltsBattery;
}

/**
 * Reads the given analog pin several times and returns the
 * average result to smooth the readings.
 * 
 * @param pin the analog pin to read
 * @return the average analog value on the given pin
 */
static int readAnalog(int pin) {
  int analogValue = 0;
  for (int i=0; i < 5; i++) {
    delay(50);
    analogValue += analogRead(pin);
  }
  return analogValue / 5;
}

/**
 * This method handles the STATE_CELL_COUNT state. It determines the batteries'
 * cell count and blinks the LED accordingly. When the battery has a valid cell 
 * count (either 2 or 3), the threshold voltages are calculated accordingly.
 */
void handleStateCellCount() {
  delay(1000);
  cellCount = determineCellCount();
  switch (cellCount) {
    case CC_2_CELLS:
      blinkPixel(300, 2, makeColor(0, 0, 255));
      DEBUG_PRINTLN("handleStateCellCount() - Battery has 2 cells");
      break;
    case CC_3_CELLS:
      blinkPixel(300, 3, makeColor(255, 0, 255));
      DEBUG_PRINTLN("handleStateCellCount() - Battery has 3 cells");
      break;
    case CC_V_TOO_LOW:
      blinkPixel(300, 4, makeColor(255, 0, 0));
      DEBUG_PRINTLN("handleStateCellCount() - Battery voltage is too low");
      break;
    case CC_V_TOO_HIGH:
      blinkPixel(300, 6, makeColor(255, 0, 0));
      DEBUG_PRINTLN("handleStateCellCount() - Battery voltage is too high");
      break;
  }
  if (CC_2_CELLS == cellCount || CC_3_CELLS == cellCount) {
    lowVoltageThreshold = cellCount * LOW_CELL_VOLTAGE;
    minVoltageThreshold = cellCount * MIN_CELL_VOLTAGE;
    DEBUG_PRINT("handleStateCellCount() - Battery voltage thresholds set to WARN: ");
    DEBUG_PRINT(lowVoltageThreshold);
    DEBUG_PRINT(" / ERROR: ");
    DEBUG_PRINT(minVoltageThreshold);
    DEBUG_PRINTLN(" mV");
  }
  delay(1500);
  currentState = STATE_CHECK_DOOR;
}

/**
 * This method handles the STATE_CHECK_VOLTAGE state. It determines the battery voltage
 * and reacts according to the set threshold and the jam door state. It triggers the LED
 * alarms if neccessary.
 */
void handleStateCheckVoltage() {
  currentState = STATE_CHECK_DOOR;
  if (CC_2_CELLS != cellCount && CC_3_CELLS != cellCount) {
    DEBUG_PRINTLN("handleStateCheckVoltage() - Skipping check, battery voltage is not within operating limits");
  } else {
    int batteryVoltage = determineBatteryVoltage();
    if (batteryVoltage < minVoltageThreshold) {
      DEBUG_PRINTLN("handleStateCheckVoltage() - [ERROR] Battery voltage below minimum threshold");    
      currentState = STATE_BATTERY_LOW;
      return;
    } else if (batteryVoltage < lowVoltageThreshold) {
      DEBUG_PRINTLN("handleStateCheckVoltage() - [WARN] Battery voltage below low threshold");
      if (doorCurrentlyOpen) {
        lightPixel(makeColor(255, 255, 0));
      } else {
        blinkPixel(500, 2, makeColor(255, 255, 0));
        delay(1500);
      }
    }
    else {
      DEBUG_PRINTLN("handleStateCheckVoltage() - Battery voltage is fine");  
      if (doorCurrentlyOpen) {
        lightPixel(makeColor(0, 255, 0));
      } else {
        lightPixel(makeColor(0, 0, 0));
      }
      
    }
  }
}

/**
 * This method handles the STATE_BATTERY_LOW state. This state is a final state,
 * meaning once the state machine reaches this point, it will not transition to 
 * any other state. This is to ensure that when a battery has gone below the safe
 * voltage limit, the user is constantly reminded that his battery is weak. Many 
 * LiPos tend to recover over a short period of time. This could fool the sketch
 * into thinking the LiPo is alright again - which it is not. As soon as current is
 * drained again from the battery, the voltage will plummet again in a very short
 * time span. This sharp drop in voltage is bad for the battery, so the user is
 * reminded to change the battery even if it recovers for a short period.
 */
void handleStateBatteryLow() {
  DEBUG_PRINTLN("handleStateBatteryLow() - Battery voltage too low. Disconnect battery and replace to reset battery monitor.");  
  if (isDoorOpen()) {
    lightPixel(makeColor(255, 0, 0));
  } else {
    blinkPixel(100, 10, makeColor(255, 0, 0));
    delay(1900);
  }
}

/**
 * This method handles the STATE_CHECK_DOOR state. Checks if the jam door is
 * open or not. It rotates {@code doorCurrentlyOpen} into {@code doorPreviouslyOpen}
 * and reads the current state into {@code doorCurrentlyOpen}. If there was a change
 * from closed to open, the method will transition to STATE_BLINK_VOLTAGE instead of
 * STATE_CHECK_VOLTAGE.
 */
void handleStateCheckDoor() {
  doorPreviouslyOpen = doorCurrentlyOpen;
  doorCurrentlyOpen = isDoorOpen();
  DEBUG_PRINT("handleStateCheckDoor() - Door is ");
  DEBUG_PRINT(doorCurrentlyOpen ? "open" : "closed");
  DEBUG_PRINT(" (was previously ");
  DEBUG_PRINT(doorPreviouslyOpen ? "open" : "closed");
  DEBUG_PRINTLN(")");
  if (doorCurrentlyOpen && !doorPreviouslyOpen) {
    currentState = STATE_BLINK_VOLTAGE;  
  } else {
    currentState = STATE_CHECK_VOLTAGE;
  }
}

/**
 * This method handles the STATE_BLINK_VOLTAGE state. It determines 
 * the battery voltage and blinks the LED once per full volt in cyan
 * and then once per 100 millivolt in purple.
 */
void handleStateBlinkVoltage() {
  int batteryVoltage = determineBatteryVoltage();
  if (batteryVoltage > 100) {
    int volts = (batteryVoltage - (batteryVoltage % 1000)) / 1000;
    int milliVolts = (batteryVoltage % 1000) / 100;
    DEBUG_PRINT("handleStateBlinkVoltage() - Will blink ");
    DEBUG_PRINT(volts);
    DEBUG_PRINT(" times for volts and ");
    DEBUG_PRINT(milliVolts);
    DEBUG_PRINTLN(" times for millivolts");
    delay(1000);
    blinkPixel(250, volts, makeColor(0, 204, 204));
    delay(volts * 500 + 500);
    blinkPixel(250, milliVolts, makeColor(178, 102, 255));
    delay(milliVolts * 500);
  }
  currentState = STATE_CHECK_VOLTAGE;
}

/**
 * This method evaluates the current state of the state machine and call the 
 * appropriate handler.
 */
void loop() {
  switch (currentState) {
    case STATE_CHECK_VOLTAGE:
      handleStateCheckVoltage();
      break;
    case STATE_CELLCOUNT:
      handleStateCellCount();
      break;
    case STATE_BATTERY_LOW:
      handleStateBatteryLow();
      break;
    case STATE_CHECK_DOOR:
      handleStateCheckDoor();
      break;
    case STATE_BLINK_VOLTAGE:
      handleStateBlinkVoltage();
      break;
  }
}
