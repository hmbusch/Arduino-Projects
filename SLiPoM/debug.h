#ifndef DEBUG_H
#define DEBUG_H

#include "settings.h"

// When we don't need any debug information printed, we might as
// well skip the calls in a central place - right here.
#ifdef DEBUG
  #define DEBUG_PRINT(x)    Serial.print (x)
  #define DEBUG_PRINTDEC(x) Serial.print (x, DEC)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTLN(x)
#endif 

#endif
