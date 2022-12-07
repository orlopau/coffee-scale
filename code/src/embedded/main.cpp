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
#include "modes/mode_calibrate.h"
#include "modes/mode_recipe.h"
#include "data/recipes.h"

#define AVERAGING_LOOPS 100

HX711LoadCell loadcell(PIN_HX711_DAT, PIN_HX711_SCK);
DefaultWeightSensor weightSensor(loadcell);
U8GDisplay display(PIN_I2C_SDA, PIN_I2C_SCL, U8G2_R1);
EmbeddedUserInput input(PIN_ENC_A, PIN_ENC_B, PIN_ENC_BTN, PIN_BUZZER);
ADCBattery battery(PIN_BAT_ADC, PIN_BAT_CHARGE_STATUS, 2, 0, 3.6, 4.2);
Stopwatch stopwatch;

void saveScale(float scale)
{
  Serial.printf("New scale: %f\n", scale);
  Serial.println("Saving scale to EEPROM...");
  EEPROM.put(EEPROM_ADDR_SCALE, scale);
  EEPROM.commit();
}

ModeDefault modeDefault(weightSensor, input, display, stopwatch);
ModeCalibration modeCalibration(weightSensor, input, display, stopwatch, saveScale);
ModeRecipes modeRecipes(weightSensor, input, display, RECIPES, RECIPE_COUNT);
Mode *modes[] = {&modeDefault, &modeRecipes, &modeCalibration};
ModeManager modeManager(modes, 3, display, input, battery);

EncoderDirection encoderDirection;

void IRAM_ATTR isr_input()
{
  input.update();
}

void setup()
{
  Serial.begin(115200);
  Serial.println("CoffeeScale v1.0.0");

  EEPROM.begin(1024);
  btStop();

  display.begin();
  weightSensor.begin();

  display.drawOpener();
  delay(3000);

  weightSensor.update();
  weightSensor.tare();

  attachInterrupt(PIN_ENC_A, isr_input, CHANGE);
  attachInterrupt(PIN_ENC_B, isr_input, CHANGE);
  attachInterrupt(PIN_ENC_BTN, isr_input, CHANGE);

  float scale;
  EEPROM.get(EEPROM_ADDR_SCALE, scale);
  if (scale <= 0 || isnan(scale))
  {
    scale = 1.0f;
  }

  Serial.printf("Existing scale: %f\n", scale);
  weightSensor.setScale(scale);
}

#ifdef PERF
unsigned int loops = 0;
unsigned long lastTime = millis();
#endif

void loop()
{
  input.update();
  weightSensor.update();
  display.update();
  modeManager.update();

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