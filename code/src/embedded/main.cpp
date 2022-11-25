#ifndef PIO_UNIT_TESTING

#include <Arduino.h>

#include "constants.h"
#include "mode.h"
#include "hx711_loadcell.h"
#include "u8g_display.h"
#include "user_input.h"
#include "adc_battery.h"
#include "mode_manager.h"

#define AVERAGING_LOOPS 100

HX711LoadCell loadcell(PIN_HX711_DAT, PIN_HX711_SCK);
U8GDisplay display(PIN_I2C_SDA, PIN_I2C_SCL, U8G2_R1);
EmbeddedUserInput input(PIN_ENC_A, PIN_ENC_B, PIN_ENC_BTN);
Stopwatch stopwatch;

void saveScale(float scale)
{
  Serial.printf("New scale: %f\n", scale);
}

ModeCalibrateLoadCell modeCalibrateLoadCell(loadcell, input, display, stopwatch, saveScale);
ModeDefault modeDefault(loadcell, input, display, stopwatch);

EncoderDirection encoderDirection;
Mode *modes[] = {&modeDefault};
const char *names[] = {"Scale"};

ModeManager modeManager(modes, names, 1, display);

void IRAM_ATTR isr_input()
{
  input.update();
}

void setup()
{
  Serial.begin(115200);
  Serial.println("CoffeeScale v1.0.0");

  attachInterrupt(PIN_ENC_A, isr_input, CHANGE);
  attachInterrupt(PIN_ENC_B, isr_input, CHANGE);
  attachInterrupt(PIN_ENC_BTN, isr_input, CHANGE);

  display.begin();
  loadcell.begin();
}

#ifdef PERF
unsigned int loops = 0;
unsigned long lastTime = millis();
#endif

void loop()
{
#ifdef PERF
  if (loops >= AVERAGING_LOOPS)
  {
    Serial.printf("Loop time: %lu\n", (millis() - lastTime) / loops);
    loops = 0;
    lastTime = millis();
  }
  loops++;
#endif

  input.update();
  loadcell.update();
  display.update();

  encoderDirection = EncoderDirection::NONE;
  if (input.isEncoderPressed())
  {
    encoderDirection = input.getEncoderDirection();
  }

  modeManager.update(static_cast<int>(encoderDirection));
}

#endif