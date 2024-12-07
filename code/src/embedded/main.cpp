#if !defined(PIO_UNIT_TESTING) && !defined(DEV_DISPLAY) && !defined(NATIVE)

#include <Arduino.h>
#include <EEPROM.h>

#include "constants.h"
#include "data/recipes.h"
#include "loadcell.h"
#include "mode.h"
#include "mode_manager.h"
#include "modes/mode_scale.h"
#include "modes/mode_espresso.h"
#include "modes/mode_calibrate.h"
#include "modes/mode_recipe.h"
#include "u8g_display.h"
#include "update.h"
#include "interface.h"
#include "battery.h"

#define AVERAGING_LOOPS 100

#define TAG "MAIN"

DefaultWeightSensor weightSensor;
U8GDisplay display(PIN_I2C_SDA, PIN_I2C_SCL, U8G2_R1);
Stopwatch stopwatch;

void saveScale(float scale)
{
  ESP_LOGI(TAG, "New scale: %f", scale);
  ESP_LOGI(TAG, "Saving scale to EEPROM...");
  EEPROM.put(EEPROM_ADDR_SCALE, scale);
  EEPROM.commit();
  weightSensor.setScale(scale);
}

ModeScale modeDefault(weightSensor, display, stopwatch);
ModeEspresso modeEspresso(weightSensor, display, stopwatch);
ModeCalibration modeCalibration(display, stopwatch, saveScale);
ModeRecipes modeRecipes(weightSensor, display, RECIPES, RECIPE_COUNT);
Mode *modes[] = {&modeDefault, &modeRecipes, &modeEspresso, &modeCalibration};
ModeManager modeManager(modes, 4, display);

Interface::EncoderDirection encoderDirection;

void IRAM_ATTR isr_input()
{
  Interface::update();
}

void setup()
{
  Serial.begin(115200);
  ESP_LOGI(TAG, "CoffeeScale starting up...");

  EEPROM.begin(2048);
  btStop();

  //////// INTERFACE //////
  Interface::begin();

  //////// BATTERY ////////
  Battery::init();

  //////// DISPLAY ////////
  display.begin();
  display.drawOpener();

  //////// WEIGHT SENSOR ////////
  LoadCell::begin();
  weightSensor.begin();

  float scale;
  EEPROM.get(EEPROM_ADDR_SCALE, scale);
  if (isnan(scale))
  {
      scale = 1.0f;
  }

  ESP_LOGI(TAG, "Existing scale: %f", scale);
  weightSensor.setScale(scale);

  float delta = 1 / scale;
  weightSensor.setAutoAveraging(abs(delta), 64);
  ESP_LOGI(TAG, "Auto averaging delta: %f", delta);

  // tare after 32 samples
  ESP_LOGI(TAG, "Taring...");
  for (int i = 0; i < 32;)
  {
      weightSensor.update();
      if (weightSensor.isNewWeight())
      {
          i++;
      }
  }
  weightSensor.tare();
  ESP_LOGI(TAG, "Tared!");

  //////// INTERRUPTS ////////
  attachInterrupt(PIN_ENC_A, isr_input, CHANGE);
  attachInterrupt(PIN_ENC_B, isr_input, CHANGE);
  attachInterrupt(PIN_ENC_BTN, isr_input, CHANGE);

  //////// UPDATES ////////
  if (digitalRead(PIN_UPDATE_FIRMWARE) == LOW)
  {
      Updater::update_firmware(display);
  }

  ESP_LOGI(TAG, "Setup finished!");
}

#ifdef PERF
unsigned int loops = 0;
unsigned long lastTime = millis();
#endif

void loop()
{
  Interface::update();
  weightSensor.update();
  display.update();
  modeManager.update();

#ifdef PERF
  if (loops >= AVERAGING_LOOPS)
  {
    ESP_LOGI(TAG, "Loop time: %lu", (millis() - lastTime) / loops
    loops = 0;
    lastTime = millis();
  }
  loops++;
#endif
}

#endif