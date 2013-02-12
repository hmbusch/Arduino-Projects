#include "ControlKnob.h"

/**
 * Initializes the instance that then can be used for monitoring a control knob.
 * The instance starts out in control mode 1 and changes the mode to the respective
 * other mode every time the control know rises above and then falls below a certain
 * threshold.
 * Using the combination of mode and knob value, an application can use one control 
 * knob to control two settings (although not at the same time).
 * 
 * @param controlPin the pin to which the control knob slider is attached
 * @param mode1SignalPin when mode 1 or control mode is enabled, this pin is pulled HIGH, e.g. for turning on a LED
 * @param mode2SignalPin when mode 2 or control mode is enabled, this pin is pulled HIGH, e.g. for turning on a LED
 * @param invertControl if true, the knob position is inverted, allowing to reverse the knob in software
 */
void ControlKnobClass::begin(uint8_t controlPin, uint8_t mode1SignalPin, uint8_t mode2SignalPin, boolean invertControl)
{
  this->signalPins[CK_MODE_1] = mode1SignalPin;
  this->signalPins[CK_MODE_2] = mode2SignalPin;
  this->isControlInverted = invertControl;
  this->controlPin = controlPin;
  this->currentControlMode = CK_MODE_1;
  this->previousControlValue = 0;
 
  pinMode(controlPin, INPUT);
  pinMode(signalPins[CK_MODE_1], OUTPUT);
  pinMode(signalPins[CK_MODE_2], OUTPUT);
 
  checkForNewControlMode();
}

/**
 * Reads the current knob position and evaluates if the change mode has been
 * enabled or disabled. If the knob leaves the control mode, the currently selected
 * mode is switched.
 * That means: by moving the knob above a certain value and back again, the user can
 * change the mode currently active. In combination with the raw control value, two 
 * different settings can be controlled.
 *
 * @return the current knob value as read by readControlValue()
 */
int ControlKnobClass::checkForNewControlMode()
{
  this->currentControlValue = readControlValue();
  
  // if we went above the threshold since the last check, enable change mode
  if (this->currentControlValue >= 1015 && this->previousControlValue < 1015)
  {
      digitalWrite(signalPins[CK_MODE_1], HIGH);
      digitalWrite(signalPins[CK_MODE_2], HIGH);
      this->isInControlChangeMode = true;     
  }
  // if went below the threshold since the last check, flip modes and leave change mode
  else if (this->currentControlValue < 1015 && this->previousControlValue >= 1015)
  {
    if (this->currentControlMode == CK_MODE_1)
    {
      this->currentControlMode = CK_MODE_2;
    }
    else
    {
      this->currentControlMode = CK_MODE_1;
    }
    this->isInControlChangeMode = false;
  }
  
  // Store the current reading as the previous reading for the next check
  this->previousControlValue = this->currentControlValue;
  
  // Signal the mode change to the user
  refreshModeDisplay();
  
  // Delay a little bit to avoid overlapping readings
  delay(10);  

}

/**
 * Reads the current position of the control knob from the controlPin and returns it. If
 * isControlInverted is true, the value is inverted by subtracting it from 1023.
 *
 * @return the control value in a range from 0 to 1023
 */
int ControlKnobClass::readControlValue()
{
  int controlValue = analogRead(this->controlPin);
  if (this->isControlInverted)
  {
    controlValue = 1023 - controlValue;
  }
  return controlValue;
}

uint8_t ControlKnobClass::getCurrentControlMode()
{
  return this->currentControlMode;
}

/**
 * Refreshes the state of the two attached signal pins to show the currently
 * selected mode.
 */
void ControlKnobClass::refreshModeDisplay()
{
  digitalWrite(signalPins[CK_MODE_1], LOW);
  digitalWrite(signalPins[CK_MODE_2], LOW);
  digitalWrite(signalPins[currentControlMode], HIGH);
}

// Offer a pre-instantiated instance to the user for convenience.
ControlKnobClass ControlKnob;
