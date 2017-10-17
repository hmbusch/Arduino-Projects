/**
 * Utility file to encapsulate the door switch debouncing logic.
 */
#ifndef DOOR_H
#define DOOR_H

/**
 * Initializes the switch and its debouncing routine. Needs to
 * be called before the door state can be queried.
 */
void initDoor();

/**
 * Determines if the jam door (or whatever switch you have hooked up)
 * is currently open or not.
 * 
 * @return {@code true} if the door is open, {@code false otherwise}
 */
boolean isDoorOpen();

#endif
