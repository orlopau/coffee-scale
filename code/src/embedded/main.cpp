#if !defined(PIO_UNIT_TESTING) && !defined(NATIVE)

#include <Arduino.h>

#include "constants.h"
#include "detectors/lift_tare.h"
#include "detectors/shot_auto_start.h"
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
#include "settings.h"
#include "ui/error_screens.h"
#include "ui/updater_screens.h"
#include "firmware_probation.h"
#include "embedded/network/firmware_check.h"
#include "embedded/blocking_screens.h"
#include "embedded/network/auto_update.h"
#include "embedded/network/wifi_setup.h"

#define AVERAGING_LOOPS 100

#define TAG "MAIN"

SampleQueue samples;
LoadCellTask loadCellTask(samples);
WeightPipeline weightPipeline(samples);
LiftTare liftTare(weightPipeline);
Stopwatch stopwatch;
FirmwareProbation probation;

void saveScale(float scale)
{
  ESP_LOGI(TAG, "New scale: %f", scale);
  Settings::setCalibration(scale);
  weightPipeline.setGramsPerCount(scale);
}

// Without its calibration the scale would weigh wrong without telling, so
// show why and wait for a click before going on with calibration 1.0.
void showSettingsError(const char *error)
{
  BlockingScreens::show([error](Canvas &canvas) { ErrorScreens::settingsUnreadable(canvas, error); });
  // a press still held or queued from start-up must not dismiss it unseen
  BlockingScreens::waitForClick();
}

// An update didn't start properly and the firmware before is back.
void showRolledBack()
{
  BlockingScreens::show([](Canvas &canvas) { UpdaterScreens::text(canvas, UPDATER_ROLLED_BACK); });
  // a press still held or queued from start-up must not dismiss it unseen
  BlockingScreens::waitForClick();
}

ModeScale modeDefault(weightPipeline, stopwatch);
ModeEspresso modeEspresso(weightPipeline, stopwatch);
ModeCalibration modeCalibration(weightPipeline, stopwatch, saveScale);
void runWifiSetup()
{
  WifiSetup::run();
}

void applyShotSettings();
const SettingsAction settingsActions[] = {{SETTINGS_WIFI, runWifiSetup}};
ModeSettings modeSettings(settingsActions, 1, applyShotSettings);
ModeRecipes modeRecipes(weightPipeline, RECIPES, RECIPE_COUNT);
Mode *modes[] = {&modeDefault, &modeRecipes, &modeEspresso, &modeCalibration, &modeSettings};
ModeManager modeManager(modes, 5, Display::canvas());
ShotAutoStart shotAutoStart(weightPipeline, modeManager, modeDefault, modeEspresso);

// at the start, and when leaving the settings, so a change needs no restart
void applyShotSettings()
{
  shotAutoStart.setAutoSwitch(Settings::shotAutoSwitch());
  shotAutoStart.setOffsetMs(Settings::shotOffsetMs());
  shotAutoStart.setTypicalShotSeconds(Settings::typicalShotSeconds());
  ESP_LOGI(TAG, "Shot detection: switch %d, offset %u ms, typical shot %f s", Settings::shotAutoSwitch(),
           (unsigned)Settings::shotOffsetMs(), Settings::typicalShotSeconds());
}

void setup()
{
  Serial.begin(115200);
  ESP_LOGI(TAG, "CoffeeScale starting up...");

  const char *settingsError = Settings::begin();
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

  float scale = Settings::getCalibration();

  ESP_LOGI(TAG, "Existing scale: %f", scale);
  weightPipeline.setGramsPerCount(scale);


  float delta = fabs(Settings::getFloat(Settings::floatSettings[Settings::AVERAGING_DELTA]));
  weightPipeline.setAutoAveraging(delta, 64);
  ESP_LOGI(TAG, "Auto averaging delta: %f g/s", delta);

  float zeroTracking = fabs(Settings::getFloat(Settings::floatSettings[Settings::ZERO_TRACKING]));
  weightPipeline.setZeroTracking(zeroTracking);
  ESP_LOGI(TAG, "Zero tracking window: %f g", zeroTracking);

  float liftDepth = Settings::liftTareDepth();
  liftTare.setDepth(liftDepth);
  ESP_LOGI(TAG, "Lift-to-tare depth: %f g", liftDepth);

  applyShotSettings();

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

  //////// UPDATES ////////
  if (digitalRead(PIN_UPDATE_FIRMWARE) == LOW)
  {
      Updater::update_firmware(scale, samples);
  }

  //////// SETTINGS ERROR ////////
  // after the updater, so holding the button at boot still reaches it
  if (settingsError != nullptr)
  {
      showSettingsError(settingsError);
  }

  //////// ROLLBACK ////////
  const bool rolledBack = FirmwareCheck::takeRolledBack();
  if (rolledBack)
  {
      showRolledBack();
  }

  //////// AUTOMATIC UPDATES ////////
  // before anything is on the platform; "Yes" installs and restarts
  AutoUpdate::askAtStart(rolledBack);

  //////// WI-FI ////////
  WifiSetup::askOnce();

  /// MODE MANAGER ////
  // the button task runs since Interface::begin(), a press during start-up must not act in the first mode
  Interface::discardClicks();
  modeManager.begin();

  //////// AUTOMATIC UPDATES ////////
  // in the background, while the modes run
  AutoUpdate::begin();

  ESP_LOGI(TAG, "Setup finished!");
}

#ifdef PERF
unsigned int loops = 0;
unsigned long lastTime = millis();
#endif

void loop()
{
  Interface::update();
  // from the mode's state of the last loop, see WeightPipeline::holdZero()
  weightPipeline.holdZero(modeManager.isMeasuring());
  weightPipeline.update();
  // a new firmware counts as working once the loop has run a while with readings
  if (probation.update(millis(), weightPipeline.newReadings().size() > 0))
  {
    FirmwareCheck::markValid();
    // an automatic update that got this far isn't rolled back any more
    AutoUpdate::clearInstalling();
  }
  // locked from the mode's state of the last loop, like holdZero()
  liftTare.update(modeManager.isMeasuring());
  // from the mode's state of the last loop, like liftTare
  shotAutoStart.update();
  modeManager.update();

#ifdef CRASH_AFTER_MS
  // only for testing rollback: PLATFORMIO_BUILD_FLAGS=-DCRASH_AFTER_MS=1000 crashes in the first loop,
  // before FirmwareProbation's 2 s are up
  if (millis() > CRASH_AFTER_MS)
  {
    abort();
  }
#endif

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