#if !defined(PIO_UNIT_TESTING) && defined(DEV_DISPLAY)

#include <Arduino.h>

#include "embedded/constants.h"
#include "mode.h"
#include "embedded/u8g_display.h"

#define TAG "MAIN-DISPLAY"

#define AVERAGING_LOOPS 100

U8GDisplay display(PIN_I2C_SDA, PIN_I2C_SCL, U8G2_R1);

// #define SWITCHER
#define ESPRESSO

void setup()
{
  Serial.begin(115200);
  ESP_LOGI(TAG, "CoffeeScale v1.0.0, Testing");

  display.begin();

  #ifdef SWITCHER
  char *switcherLabels[10];
  for (int i = 0; i < 10; i++)
  {
    switcherLabels[i] = new char[20];
    sprintf(switcherLabels[i], "V60 %d  Element", i);
  }

  for (;;)
  {
    for (int i = 0; i < 10; i++)
    {
      display.switcher(i, 10, const_cast<const char **>(switcherLabels));
      delay(500);
    }
  }
  #endif

  #ifdef ESPRESSO
  display.espressoShot(4000, 26000, 5000, 18000);
  #endif
}

void loop()
{
}

#endif