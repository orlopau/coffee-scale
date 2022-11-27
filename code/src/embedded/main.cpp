#ifndef PIO_UNIT_TESTING

#include <Arduino.h>
#include <EEPROM.h>

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
  Serial.println("Saving scale to EEPROM...");  
  EEPROM.put(EEPROM_ADDR_SCALE, scale);
  EEPROM.commit();
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
  EEPROM.begin(1024);

  attachInterrupt(PIN_ENC_A, isr_input, CHANGE);
  attachInterrupt(PIN_ENC_B, isr_input, CHANGE);
  attachInterrupt(PIN_ENC_BTN, isr_input, CHANGE);

  float scale;
  EEPROM.get(EEPROM_ADDR_SCALE, scale);
  if (scale == 0 || isnan(scale))
  {
    scale = 1.0f;
  }

  Serial.printf("Existing scale: %f\n", scale);
  loadcell.setScale(scale);

  display.begin();

  // display.switcher("V60 Tetsu Kasuya", 1, 6,
  //                  "V60 Tetsu Kasuya\nV60 James Hoffmann\nAeropress James Hoffmann\nKalita Recipe\nSpecial Recipe\nSomeething\nTake that recipe!");
}

#ifdef PERF
unsigned int loops = 0;
unsigned long lastTime = millis();
#endif

void loop()
{
  input.update();
  loadcell.update();
  display.update();

  modeCalibrateLoadCell.update();

  // encoderDirection = EncoderDirection::NONE;
  // if (input.isEncoderPressed())
  // {
  //   encoderDirection = input.getEncoderDirection();
  // }

  // modeManager.update(static_cast<int>(encoderDirection));

#ifdef PERF
  if (loops >= AVERAGING_LOOPS)
  {
    Serial.printf("Loop time: %lu\n", (millis() - lastTime) / loops);
    loops = 0;
    lastTime = millis();
  }
  loops++;
#endif
}

#endif