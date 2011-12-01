#define PIN_DATA 10
#define PIN_CLOCK 11
#define PIN_LATCH 12

#define MAX_VALUE 16

byte segmentPins[2] = {3, 4};
byte segmentCount = 2;
byte segmentValues[2] = {0, 0};
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
  
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);
  for(int i = 0; i < segmentCount; i++)
  {
    pinMode(segmentPins[i], OUTPUT);
  }
}

ISR(TIMER2_OVF_vect)
{
  displaySegment(currentSegment);
  
  currentSegment++;
  if (currentSegment >= segmentCount)
  {
    currentSegment = 0;
  }
};

void displaySegmentValue(int value)
{
  digitalWrite(PIN_LATCH, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, value);
  digitalWrite(PIN_LATCH, HIGH);
}

void displaySegment(byte which)
{
  toggleSegment(which);
  displaySegmentValue(segmentValues[which]);
  delay(1);
  displaySegmentValue(0);
}

void toggleSegment(byte which)
{
  for(int i = 0; i < segmentCount; i++)
  {
    if(i == which)
    {
      digitalWrite(segmentPins[i], HIGH);
    }
    else
    {
      digitalWrite(segmentPins[i], LOW);
    }
  }
}

void loop()
{
  for(int i = 0; i <= MAX_VALUE; i++)
  {
    updateSegmentValues(i);
    delay(100);
  }
  delay(5000);
  for(int i = MAX_VALUE; i >= 0; i--)
  {
    updateSegmentValues(i);
    delay(100);
  }
  delay(5000);
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




