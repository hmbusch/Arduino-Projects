#include <InputDebounce.h>
#include "door.h"
#include "settings.h"
#include "debug.h"

/**
 * The debounce object for the door switch.
 */
static InputDebounce doorSwitch; // not enabled yet, setup has to be called later

/**
 * Initializes the switch and its debouncing routine. Needs to
 * be called before the door state can be queried.
 */
void initDoor() {
  pinMode(PIN_DOOR, INPUT);
  doorSwitch.setup(PIN_DOOR, 20, InputDebounce::PIM_EXT_PULL_DOWN_RES);
}

/**
 * Determines if the jam door (or whatever switch you have hooked up)
 * is currently open or not.
 * 
 * @return {@code true} if the door is open, {@code false otherwise}
 */
boolean isDoorOpen() {
    unsigned int onTime = doorSwitch.process(millis());
    return !(onTime > 0);
}

