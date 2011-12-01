#define PIN_DATA 10
#define PIN_CLOCK 11
#define PIN_LATCH 12

void setup()
{
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);
}

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
