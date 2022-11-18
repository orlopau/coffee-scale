#ifndef PIO_UNIT_TESTING

#include "Arduino.h"
#include "RotaryEncoder.h"
#include "constants.h"

void setupPins()
{
}

RotaryEncoder encoder(PIN_ENC_A, PIN_ENC_B, RotaryEncoder::LatchMode::FOUR3);

void setup()
{
  Serial.begin(115200);
  setupPins();
}

void loop()
{
  static int pos = 0;
  encoder.tick();

  int newPos = encoder.getPosition();
  if (pos != newPos)
  {
    Serial.print("pos:");
    Serial.print(newPos);
    Serial.print(" dir:");
    Serial.println((int)(encoder.getDirection()));
    pos = newPos;
  } // if
}

#endif