#include "Arduino.h"
#include "RotaryEncoder.h"
#include "battery.h"
#include "constants.h"

void setupPins()
{
  pinMode(PIN_BAT_CHARGE_STATUS, INPUT_PULLUP);
  pinMode(PIN_V_BAT, INPUT);

  pinMode(PIN_BUZZER, OUTPUT);
}

RotaryEncoder encoder(PIN_ENC_A, PIN_ENC_B, RotaryEncoder::LatchMode::FOUR3);

void setup()
{
  Serial.begin(115200);
  setupPins();
}

void loop()
{
  // Serial.printf("Battery voltage: %f, ", batteryVoltage());
  // Serial.printf("percentage: %f, ", batteryPercentage());
  // Serial.printf("charging: %d", batteryCharging());
  // Serial.print("\n");
  // delay(1000);

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