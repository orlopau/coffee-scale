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
#include "modes/mode_espresso.h"
#include "modes/mode_recipe.h"
#include "modes/mode_scale.h"
#include "u8g_display.h"
#include "update.h"
#include "user_input.h"

#define AVERAGING_LOOPS 100

#define TAG "MAIN"

HX711LoadCell loadcell(PIN_HX711_DAT, PIN_HX711_SCK);
DefaultWeightSensor weightSensor(loadcell);
U8GDisplay display(PIN_I2C_SDA, PIN_I2C_SCL, U8G2_R1);
EmbeddedUserInput input(PIN_ENC_A, PIN_ENC_B, PIN_ENC_BTN, PIN_BUZZER);
ADCBattery battery(PIN_BAT_ADC, PIN_BAT_CHARGE_STATUS, 2 * 0.996116504854369, 0, 3, 4.2);
Stopwatch stopwatch;

void saveScale(float scale)
{
    ESP_LOGI(TAG, "New scale: %f", scale);
    ESP_LOGI(TAG, "Saving scale to EEPROM...");
    EEPROM.put(EEPROM_ADDR_SCALE, scale);
    EEPROM.commit();
    weightSensor.setScale(scale);
}

ModeScale modeDefault(weightSensor, input, display, stopwatch);
ModeEspresso modeEspresso(weightSensor, input, display, stopwatch);
ModeCalibration modeCalibration(loadcell, input, display, stopwatch, saveScale);
ModeRecipes modeRecipes(weightSensor, input, display, RECIPES, RECIPE_COUNT);
Mode *modes[] = {&modeDefault, &modeRecipes, &modeEspresso, &modeCalibration};
ModeManager modeManager(modes, 4, display, input, battery);

EncoderDirection encoderDirection;

void IRAM_ATTR isr_input() { input.update(); }

void IRAM_ATTR isr_loadcell()
{
    weightSensor.update();
}

void input_loop(void *params)
{
    ESP_LOGI(TAG, "Started input loop.");
    for (;;)
    {
        weightSensor.update();
        input.update();
        modeManager.update();
        display.update();
    }
}

void setup()
{
    Serial.begin(115200);
    ESP_LOGI(TAG, "CoffeeScale starting up...");

    EEPROM.begin(2048);
    btStop();

    //////// DISPLAY ////////
    display.begin();
    display.drawOpener();
    display.update();

    //////// WEIGHT SENSOR ////////
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

    ESP_LOGI(TAG, "Taring...");
    for (int i = 0; i < 16;)
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
        Updater::update_firmware(display, input);
    }

    //////// SCHEDULE TAKS ////////
    int code = xTaskCreatePinnedToCore(input_loop, "INPUT", 64 * 1024, NULL, 10, NULL, ARDUINO_RUNNING_CORE);
    if (code != pdPASS)
    {
        ESP_LOGE(TAG, "Task couldnt be created: %d", code);
    }

    ESP_LOGI(TAG, "Setup finished!");
}

void loop()
{
    // NOP
}

#endif