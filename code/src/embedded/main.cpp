#if !defined(PIO_UNIT_TESTING) && !defined(DEV_DISPLAY) && !defined(NATIVE)

#include <Arduino.h>
#include <EEPROM.h>

#include "adc_battery.h"
#include "constants.h"
#include "data/recipes.h"
#include "hx711_loadcell.h"
#include "mode.h"
#include "mode_manager.h"
#include "modes/mode_calibrate.h"
#include "modes/mode_recipe.h"
#include "u8g_display.h"
#include "update.h"
#include "user_input.h"

#define AVERAGING_LOOPS 100

HX711LoadCell loadcell(PIN_HX711_DAT, PIN_HX711_SCK);
DefaultWeightSensor weightSensor(loadcell);
U8GDisplay display(PIN_I2C_SDA, PIN_I2C_SCL, U8G2_R1);
EmbeddedUserInput input(PIN_ENC_A, PIN_ENC_B, PIN_ENC_BTN, PIN_BUZZER);
ADCBattery battery(PIN_BAT_ADC, PIN_BAT_CHARGE_STATUS, 2 * 0.996116504854369, 0, 3, 4.2);
Stopwatch stopwatch;

void saveScale(float scale)
{
  Serial.printf("New scale: %f\n", scale);
  Serial.println("Saving scale to EEPROM...");
  EEPROM.put(EEPROM_ADDR_SCALE, scale);
  EEPROM.commit();
  weightSensor.setScale(scale);
}

ModeDefault modeDefault(weightSensor, input, display, stopwatch);
ModeCalibration modeCalibration(loadcell, input, display, stopwatch, saveScale);
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

  EEPROM.begin(2048);
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

  float delta = 1 / scale;
  weightSensor.setAutoAveraging(delta, 64);
  Serial.printf("Auto averaging delta: %f\n", delta);

  if (digitalRead(PIN_UPDATE_FIRMWARE) == LOW)
  {
      Updater::update_firmware(display);
  }

  Serial.print("Setup finished!\n");
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