const uint8_t ledPins[] = {2, 3, 4, 5, 6, 7, 8, 9};

const uint8_t ledCount = 8;

const uint8_t fadePin = 11;

const uint8_t controlPin = 3;

const uint8_t frames[] = {B10000001, B01000010, B00100100, B00011000};

const boolean invertControl = true;

uint8_t currentFrame = 0;

void setup()
{
  for(uint8_t i = 0; i < ledCount; i++)
  {
    pinMode(ledPins[i], OUTPUT);
  }
  pinMode(fadePin, OUTPUT);
  pinMode(controlPin, INPUT);
}

void loop()
{
  int controlValue = readControlValue();
  int fadeValue = map(controlValue, 0, 1023, 0, 255);
  analogWrite(fadePin, fadeValue);
  
  if(currentFrame > 3)
  {
    currentFrame = 0;
  }
  for(uint8_t i = 0; i < 8; i++)
  {
    digitalWrite(ledPins[i], LOW);
    digitalWrite(ledPins[i], bitRead(frames[currentFrame], i));
  }
  currentFrame++;
  delay(500);  
}

int readControlValue()
{
  int controlValue = analogRead(controlPin);
  if (invertControl)
  {
    controlValue = 1023 - controlValue;
  }
  return controlValue;
}
