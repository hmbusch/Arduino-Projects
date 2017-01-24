/*
 * Sample program for use with the LED-Matrix Controller.
 * https://github.com/hmbusch/Arduino-Projects/LED-Matrix
 *
 * Hendrik Busch, http://www.icanmakeit.de/, 2011-2017
 * Released under a Creative Commons Attribution-NonCommercial-ShareAlike 3.0 
 *
 * This program consists of more or less two different parts.
 * One part does the text scrolling by recalculating the contents
 * of the 8x8 pixel viewport and storing the current frame in an
 * array.
 * The second part is controlled by an internal timer and just draws
 * the contents of the array onto the matrix. 
 * Drawing and scrolling is therefor independent of each other and we
 * do not run into sync problems.
 *
 * Go to http://www.youtube.com/watch?v=g0VkgF8Dbhs to see it in action.
 * 
 * This sketch heavily relies on code originally posted here:
 *
 * http://timewitharduino.blogspot.com/2010/03/scrolling-text-on-dual-rg-matrix-shield.html
 *
 * Version history:
 * 
 *   2011/03/24 - Initial checkin / release
 *   2017/01/24 - Updated sketch 
 *                  - now works with latest Arduino
 *                  - ditched all the proprietary timer stuff in favor of Timer1 library
 */

// The sketch uses TimerOne to trigger the line refreshs with a ~60 Hz
// refresh rate. Please note that this library might not run on every
// Arduino compatible board, e.g. the ESP8266.
#include <TimerOne.h>

// Include the sprites file that holds all the displayable characters
#include "sprites.h"

// Define the pins for the shift register
const int shiftDataPin = 11;
const int shiftClockPin = 12;
const int shiftLatchPin = 13;

// Define the pins for the decade counter
const int decadeClockPin = 7;
const int decadeResetPin = 6;

// This variable holds the current frame that the matrix
// will display. The scrolling part of the program just 
// manipulates this array while the timed drawing part uses
// the array to draw the frame
int screenState[8] = {0};

// These four byte make up a small sprite buffer for the
// scrolling process
byte char1 = 0;
byte char2 = 0;
byte char3 = 0;
byte char4 = 0;

// This is the time delay (in milliseconds) between the scrolling
// of two columns. The lower the value, the faster the scrolling.
// You might as well call it the scroll speed.
const int waitTime = 150;

// This is the text that will be displayed by scrolling. Declaring this
// as character array is not optimal as it is limited in size. I might
// switch this to a String object later on.
char msgBuffer[] = " TAKE ME OUT, INTO THE BLACK, TELL THEM I AIN'T COMING BACK - BURN THE LAND AND BOIL THE SEA, YOU CAN'T TAKE THE SKY FROM ME    ";

// This is the current scrolling position in the message string
byte msgBufferPos  = 0;

// This is the length of the message string
byte msgBufferSize = 0;


byte soft_prescaler = 0;

// This is the index of the currently displayed row
byte activeRow = 0;

/*
 * Sets up the pins and configures the timer. Initially, this sketch used
 * the ATMega hardware timers, which complicated things quite a bit. I removed
 * these parts of the code and replaced them with the TimerOne library (refer
 * to https://www.pjrc.com/teensy/td_libs_TimerOne.html for more information).
 * The timer is set to trigger every 2000 microseconds which gives a nice and 
 * steady display without flickering.
 */
void setup(){
  // Initialize the Timer1 with 2000 microsecond periods.
  Timer1.initialize(2000);
  Timer1.attachInterrupt(displayActiveRow);

  // define outputs for serial shift registers
  pinMode(shiftClockPin, OUTPUT);
  pinMode(shiftLatchPin, OUTPUT);
  pinMode(shiftDataPin, OUTPUT);

  pinMode(decadeClockPin, OUTPUT);
  pinMode(decadeResetPin, OUTPUT);

  // reset the display for a fresh start
  resetDisplay();

  // determine the length of the message string
  msgBufferSize = strlen(msgBuffer);

  // Start displaying lines
  Timer1.start();
}

/*
 * Resets the internal screen state and the character buffer by filling them
 * with zeros. The timer based redrawing mechanism will then blank the display.
 */
void resetDisplay()
{
  for (byte i = 0; i < 8; i++)  
  {
    screenState[i] = 0;
  }
  char1 = 0;
  char2 = 0;
  char3 = 0;
  char4 = 0;

  // reset the buffer pointers;
  msgBufferPos  = 0;
}

/*
 * Every loop cycle causes the display to scroll one character from
 * the message string. If it reaches the end of the messages, it begins
 * again from the start.
 */
void loop() {
  displayAndScroll(msgBuffer[msgBufferPos]);
  msgBufferPos++;
  if (msgBufferPos >= msgBufferSize)  msgBufferPos = 0;
}

/*
 * Determines the current row, reads its content from the screenState
 * and draws it onto the matrix.
 */
void displayActiveRow()
{
  // Set the next row by simply incrementing the value and the calculating
  // modulo 8. This takes care of the overflow, i.e. a values of 8 will 
  // simply flip back to 0
  activeRow = (activeRow+1) % 8;

  // when we have reached the first row, we need to reset the decade
  // counter in order to skip output 9 and 10 that we don't use.
  if (activeRow == 0)
  {
    pulse(decadeResetPin);
  }

  // draws the row onto the screen
  drawRow(screenState[activeRow]);

  // Draws a blank line immediately after drawing the real line. This
  // may seem a bit strange but it helps preventing 'ghost images' on the
  // display. And besides: redrawing happens so fast that actually no one
  // notices. Feel free to comment out this line to see the 'ghost' effect
  // for yourself
  drawRow(0);

  // switch to the next row on the decade counter
  pulse(decadeClockPin);
}

/*
 * Draws a single row onto the display.
 */
void drawRow(byte bytes)
{
  digitalWrite(shiftLatchPin, LOW);

  // if output seems mirrored of upside down, change MSBFIRST TO LSBFIRST
  shiftOut(shiftDataPin, shiftClockPin, MSBFIRST, bytes);
  digitalWrite(shiftLatchPin, HIGH);  
}

/*
 * Determines the sprite for the next character and pushes in into the
 * sprite buffer. The method looks a bit ugly, but it works ;-)
 */
void displayAndScroll(char crtChar)
{
  switch (crtChar)
  {
  case ' ':
    char1 = char2;
    char2 = char3;
    char3 = char4;
    char4 = 0;
    break;

  case 'A':  
  case 'B':  
  case 'C':  
  case 'D':  
  case 'E':  
  case 'F':
  case 'G':  
  case 'H':  
  case 'I':  
  case 'J':  
  case 'K':  
  case 'L':
  case 'M':  
  case 'N':  
  case 'O':  
  case 'P':  
  case 'Q':  
  case 'R':
  case 'S':  
  case 'T':  
  case 'U':  
  case 'V':  
  case 'W':  
  case 'X':
  case 'Y':  
  case 'Z':
    char1 = char2;
    char2 = char3;
    char3 = char4;
    char4 = crtChar - 'A' + 1;
    break;

  case '0':  
  case '1':  
  case '2':  
  case '3':  
  case '4':
  case '5':  
  case '6':  
  case '7':  
  case '8':  
  case '9':
    char1 = char2;
    char2 = char3;
    char3 = char4;
    char4 = crtChar - '0' + 27;
    break;

  case '.':
    // definition of the bitmaps for digits start at index 37;
    char1 = char2;
    char2 = char3;
    char3 = char4;
    char4 = 37;  // dot has index 37 in the character definition array;
    break;

  case ',':
    char1 = char2;
    char2 = char3;
    char3 = char4;
    char4 = 38;  // comma has index 38 in the character definition array;
    break;

  case '!':
    char1 = char2;
    char2 = char3;
    char3 = char4;
    char4 = 39;  // exclamation has index 39 in the character definition array;
    break;

  case ':':
    char1 = char2;
    char2 = char3;
    char3 = char4;
    char4 = 40;  // colon has index 40 in the character definition array;
    break;

  case '-':
    char1 = char2;
    char2 = char3;
    char3 = char4;
    char4 = 41; // hyphen has index 41 in the character definition array;
    break;    

  case '\'':
    char1 = char2;
    char2 = char3;
    char3 = char4;
    char4 = 42; // dash has index 42 in the character definition array;
    break;       
  } 

  // Updates the screen state with the next four sprites
  setScreenMem(sprites[char1], sprites[char2], sprites[char3], sprites[char4]);
}

/*
 * Updates the screen state with the next four sprites by assembling 8 large rows
 * and scrolling those on the display.
 */
void setScreenMem(byte sprite1[8], byte sprite2[8], byte sprite3[8], byte sprite4[8])
{
  // This will be the row array. Each row consists of the concatenation of the
  // corresponding row of each sprite
  unsigned int row[8] = {0};

  // for each row;
  for (byte i = 0; i < 8; i++)
  {
    // Take one row of the sprite at at time and shift by one to the right
    // to remove the last column from the sprite which will be empty anyway
    // so that the gaps between the character don't appear so large. (Refer
    // to the sprite definitions to really so that the last column of each
    // sprite is always empty and can be removed therefor)
    byte c1 = sprite1[i] >> 1;
    byte c2 = sprite2[i] >> 1;
    byte c3 = sprite3[i] >> 1;
    byte c4 = sprite4[i] >> 1;
    
    // assemble one long row from the individual sprite rows and offset them
    // 5 pixels to each other (each sprite has an effective width of 5 pixels)
    row[i] = ((((((unsigned long) c1 << 5) + c2) << 5) + c3) << 5) + c4;
  }  

  // scroll 5 times to the left (5 being the width of a char, as defined);
  // Wait a certain display time between scrolls
  // Redrawing is done using the defined interrupt and thus idependently of scrolling
  for (byte x = 1; x <= 5; x++)
  {
    // for each row;
    for (byte i = 0; i < 8; i++)
    {
      screenState[i] = row[i] >> (5-x);
    }
    delay(waitTime);
  }
}

/*
 * 'Pulses' the given pin by switching it high and then low once
 * in very short sequence. (20 microseconds between state changes).
 * If your ICs are too slow to respond to that correctly, increase
 * the delay between the changes).
 */
void pulse(int pin)
{
  delayMicroseconds(20);
  digitalWrite(pin, HIGH);
  delayMicroseconds(20);
  digitalWrite(pin, LOW);
  delayMicroseconds(20);
}
