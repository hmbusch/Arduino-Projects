#define PIN_DATA 10
#define PIN_CLOCK 11
#define PIN_LATCH 12

#define PIN_VCC 3

int state = LOW;
int softPrescaler = 0;

void setup()
{
  // Rechnung: 16 MHz / 256 Prescaler / 256 Overflow Limit = ~244 Hz
  // davon jeder 3 ausgeführt = ~81 Hz
  TCCR2A = 0;           // normal operation for timer 2
  TCCR2B = (1 << CS22 | 1 << CS21);   // select prescaler 1024
  TIMSK2 = (1<<TOIE2);  // enable overflow interrupt
  
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_VCC, OUTPUT);
}

ISR(TIMER2_OVF_vect)
{
  if (softPrescaler < 1)
  {
    softPrescaler += 1;
  }
  else
  {
    if (LOW == state)
    {
      state = HIGH;
    }
    else
    {
      state = LOW;
    }
    digitalWrite(PIN_VCC, state);
    softPrescaler = 0;
  }
};

void loop()
{
  digitalWrite(PIN_LATCH, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, 255);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, 255);
  digitalWrite(PIN_LATCH, HIGH);
  delay(10000);
  digitalWrite(PIN_LATCH, LOW);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, 0);
  shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, 0);
  digitalWrite(PIN_LATCH, HIGH);
  delay(10000);
}
