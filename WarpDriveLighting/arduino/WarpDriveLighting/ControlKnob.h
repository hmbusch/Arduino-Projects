// Pre-processor directive to ensure that this header file is only
// included once, no matter how often it is used in an include statement.
#ifndef ICMI_ControlKnob_h
#define ICMI_ControlKnob_h

// In order to use the well known Arduino commands, we need to include
// the main Arduino header file. This will only work with Arduino 1.0
// or greater, because the header file was named differently before the
// 1.0 release.
#include <Arduino.h>

#define CK_MODE_1 0
#define CK_MODE_2 1

class ControlKnobClass
{
public:
  
  void begin(uint8_t controlPin, uint8_t mode1SignalPin, uint8_t mode2SignalPin, boolean invertControl);
  
  int checkForNewControlMode();
  
  uint8_t getCurrentControlMode();
  
private:

  void refreshModeDisplay();
  int readControlValue();

  boolean isControlInverted;
  boolean isInControlChangeMode;
  int previousControlValue;
  int currentControlValue;
  uint8_t currentControlMode;
  uint8_t controlPin;
  uint8_t signalPins[2];
  
};

extern ControlKnobClass ControlKnob;

#endif
