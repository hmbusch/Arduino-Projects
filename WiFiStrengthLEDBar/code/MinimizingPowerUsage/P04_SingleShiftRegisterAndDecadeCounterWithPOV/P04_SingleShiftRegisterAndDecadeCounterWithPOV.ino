/*==================================================================
 * Value display using a single shift register and a decade counter
 *==================================================================
 *
 * This sketch drives a display with segments of 8 LEDs that displays 
 * a numerical value by lighting a corresponding amount of LEDs.
 *
 */ 

// Defines the pins for the shift register and the decade counter
#define PIN_SHIFT_DATA 10
#define PIN_SHIFT_CLOCK 11
#define PIN_SHIFT_LATCH 12
#define PIN_DECADE_CLOCK 6
#define PIN_DECADE_RESET 5

// Defines the maximum numerical value that the current setup can display.
// It is calculated using this formula: (number of LEDs per segment) x (segment count)
#define MAX_VALUE 16

// the number of 8-LED segments the setup consists of
byte segmentCount = 2;

// the display status of each segment
byte segmentValues[2] = {0, 0};

// the segment that is currently lit
int currentSegment = 0;

void setup()
{
  // Calculation: 16 MHz / 1024 Prescaler / 48 Preload (i.e. 207 Overflow Limit) = ~75 Hz
  // 16,000,000 : 1024 = 15625
  // 15625 : 207 = 75,48...
  TCCR2A = 0;                       // normal operation for timer 2
  TCCR2B = (1<<CS22) | (1<<CS20);   // select prescaler 1024
  TCNT2 = 48;                       // set preload to 48
  TIMSK2 = (1<<TOIE2);              // enable overflow interrupt
  
  pinMode(PIN_SHIFT_DATA, OUTPUT);
  pinMode(PIN_SHIFT_CLOCK, OUTPUT);
  pinMode(PIN_SHIFT_LATCH, OUTPUT);
  pinMode(PIN_DECADE_RESET, OUTPUT);
  pinMode(PIN_DECADE_CLOCK, OUTPUT);
  
  delay(5000);
}

ISR(TIMER2_OVF_vect)
{
  displayNextSegment();
};

void displaySegmentValue(int value)
{
  digitalWrite(PIN_SHIFT_LATCH, LOW);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, value);
  digitalWrite(PIN_SHIFT_LATCH, HIGH);
}

/*
 * Switches supply power to the next display segment by clocking the 
 * decade counter. If the last segment is reached, the cycle starts
 * over with the first segment.
 */
void displayNextSegment()
{
  // This modulo division ensures that the segment index is always 0 <= segmentIndex < segmentCount.
  // It also causes the first cycle to be unsychronized because the decade displays the first segment, whereas
  // the shift register already displays the next one. Due to the high speed refresh and the reset of the
  // decade counter once per cycle, register and counter will be in sync again after the first cycle.
  currentSegment = (currentSegment + 1) % segmentCount;

  // We need to reset the decade once per cycle  
  if(currentSegment == 0)
  {
    pulse(PIN_DECADE_RESET);
  }
  
  // Displays the value and then turns all LEDs of to prevent a ghosting effect
  displaySegmentValue(segmentValues[currentSegment]);
  delay(1);
  displaySegmentValue(0);
  delay(1);
  
  pulse(PIN_DECADE_CLOCK);
}

void loop()
{
  for(int i = 0; i <= MAX_VALUE; i++)
  {
    updateSegmentValues(i);
    delay(50);
  }
  delay(100);
  for(int i = MAX_VALUE; i >= 0; i--)
  {
    updateSegmentValues(i);
    delay(50);
  }
  delay(100);
}

void updateSegmentValues(int value)
{
  // Start with bit 0 in segment 0
  int segmentIndex = 0;
  int bitIndex = 0;
  
  // clear the whole segment 0
  segmentValues[segmentIndex] = 0;
  
  // iterate on the value and set the corresponding number of bits
  for (int i = 1; i <= value; i++)
  {
    bitSet(segmentValues[segmentIndex], bitIndex);
    bitIndex++;
    // if one segment has been completed, reset the bit index and switch to the next segment
    if (bitIndex >= 8)
    {
      bitIndex = 0;
      segmentIndex++;
      // reset the next segment if there is a next one
      if (segmentIndex < segmentCount)
      {
        segmentValues[segmentIndex] = 0;
      }
    }  
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




