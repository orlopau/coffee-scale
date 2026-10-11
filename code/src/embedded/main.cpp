#if !defined(PIO_UNIT_TESTING) && !defined(NATIVE)

#include <Arduino.h>
#include <EEPROM.h>

#include "constants.h"
#include "data/recipes.h"
#include "embedded/loadcell_task.h"
#include "embedded/sample_queue.h"
#include "weighing/weight_pipeline.h"
#include "mode.h"
#include "mode_manager.h"
#include "modes/mode_scale.h"
#include "modes/mode_espresso.h"
#include "modes/mode_calibrate.h"
#include "modes/mode_recipe.h"
#include "modes/mode_settings.h"
#include "display.h"
#include "update.h"
#include "interface.h"
#include "battery.h"

#define AVERAGING_LOOPS 100

#define TAG "MAIN"

SampleQueue samples;
LoadCellTask loadCellTask(samples);
WeightPipeline weightPipeline(samples);
Stopwatch stopwatch;

void saveScale(float scale)
{
  ESP_LOGI(TAG, "New scale: %f", scale);
  ESP_LOGI(TAG, "Saving scale to EEPROM...");
  EEPROM.put(EEPROM_ADDR_SCALE, scale);
  EEPROM.commit();
  weightPipeline.setGramsPerCount(scale);
}

ModeScale modeDefault(weightPipeline, stopwatch);
ModeEspresso modeEspresso(weightPipeline, stopwatch);
ModeCalibration modeCalibration(weightPipeline, stopwatch, saveScale);
ModeSettings modeSettings;
ModeRecipes modeRecipes(weightPipeline, RECIPES, RECIPE_COUNT);
Mode *modes[] = {&modeDefault, &modeRecipes, &modeEspresso, &modeCalibration, &modeSettings};
ModeManager modeManager(modes, 5, Display::canvas());

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
  Display::begin();
  Display::drawOpener();

  //////// WEIGHT SENSOR ////////
  samples.begin();
  loadCellTask.begin();

  float scale;
  EEPROM.get(EEPROM_ADDR_SCALE, scale);
  if (isnan(scale))
  {
      scale = 1.0f;
  }

  ESP_LOGI(TAG, "Existing scale: %f", scale);
  weightPipeline.setGramsPerCount(scale);


  float delta = fabs(Settings::getFloat(Settings::floatSettings[Settings::AVERAGING_DELTA]));
  weightPipeline.setAutoAveraging(delta, 64);
  ESP_LOGI(TAG, "Auto averaging delta: %f g/s", delta);

  // tare after 32 samples
  ESP_LOGI(TAG, "Taring...");
  for (size_t readings = 0; readings < 32;)
  {
      delay(10);
      weightPipeline.update();
      readings += weightPipeline.newReadings().size();
  }
  weightPipeline.tare();
  ESP_LOGI(TAG, "Tared!");

  //////// INTERRUPTS ////////
  attachInterrupt(PIN_ENC_A, isr_input, CHANGE);
  attachInterrupt(PIN_ENC_B, isr_input, CHANGE);
  attachInterrupt(PIN_ENC_BTN, isr_input, CHANGE);

  //////// UPDATES ////////
  if (digitalRead(PIN_UPDATE_FIRMWARE) == LOW)
  {
      Updater::update_firmware(scale, samples);
  }

  /// MODE MANAGER ////
  modeManager.begin();

  ESP_LOGI(TAG, "Setup finished!");
}

#ifdef PERF
unsigned int loops = 0;
unsigned long lastTime = millis();
#endif

void loop()
{
  Interface::update();
  weightPipeline.update();
  modeManager.update();

#ifdef PERF
  if (loops >= AVERAGING_LOOPS)
  {
    ESP_LOGI(TAG, "Loop time: %lu", (millis() - lastTime) / loops);
    loops = 0;
    lastTime = millis();
  }
  loops++;
#endif
}

#endif