/*
 * Sample program for use with the LED-Matrix Controller.
 * https://github.com/hmbusch/Arduino-Projects/LED-Matrix
 *
 * Hendrik Busch, http://www.icanmakeit.de/, 2011
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
 */

// Include the sprites file that holds all the displayable characters
#include "sprites.h"

// Define the pins for the shift register
int shiftDataPin = 11;
int shiftClockPin = 12;
int shiftLatchPin = 13;

// Define the pins for the decade counter
int decadeClockPin = 7;
int decadeResetPin = 6;

// This variable holds the current frame that the matrix
// will display. The scrolling part of the program just 
// manipulates this array while the timed drawing part uses
// the array to draw the frame
int screenState[8] = {
  0};

// These four byte make up a small sprite buffer for the
// scrolling process
byte char1 = 0;
byte char2 = 0;
byte char3 = 0;
byte char4 = 0;

// This is the time delay (in milliseconds) between the scrolling
// of two columns. The lower the value, the faster the scrolling.
// You might as well call it the scroll speed.
int waitTime = 150;

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
 * Sets up the pins and configures the timer by setting the hardware prescaler
 * options.
 * 
 * The hardware prescaler configured in setup() yields a frequency of 7812 Hz
 * which is to much as we need around 60 Hz which cannot be achieved by using
 * only hardware prescaling. The trick is to use a 'software prescaler' which
 * allows us to skip 15 timer events and use only every 16th. By that we yield
 * 488 timer events per second during which we display one row at a time.
 * So it takes 488/8 timer events to redraw the whole display which is roughly
 * 60 Hz and enough for the POV effect.
 * 
 * This matter is a little complicated and accesses some functions of the ATmega
 * chip directly. German users may consult 
 * http://www.uni-koblenz.de/~physik/informatik/MCU/Timer.pdf
 * for better understanding, English users may want to consider the official
 * documentation at http://www.atmel.com/dyn/resources/prod_documents/doc2505.pdf
 * or use Google for other tutorials.
 */
void setup(){
  // Calculation for timer 2
  // 16 MHz / 8 = 2 MHz (prescaler 8)
  // 2 MHz / 256 = 7812 Hz
  // soft_prescaler = 15 ==> 488 updates per second (every 16th out of 7812 Hz)
  // 488 / 8 rows ==> 61 Hz for the complete display
  TCCR2A = 0;           // normal operation for timer 2
  TCCR2B = (1<<CS21);   // select prescaler 8
  TIMSK2 = (1<<TOIE2);  // enable overflow interrupt

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
}

/*
 * This is the method that will be called on a timer event. The first 15 events
 * are simply counted, the 16th event is then used to draw the next row onto the
 * display and to reset the counter. Thus, every 16th timer event leads to the
 * drawing of a row.
 */
ISR(TIMER2_OVF_vect)
{
  soft_prescaler++;
  if (soft_prescaler == 15)
  {
    // display the next row
    displayActiveRow();
    soft_prescaler = 0;
  }
};

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






