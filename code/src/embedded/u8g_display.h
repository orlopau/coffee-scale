#ifndef NATIVE

#pragma once

#include <U8g2lib.h>

#include "user_interface.h"

class U8GDisplay : public Display
{
public:
    U8GDisplay(const uint8_t pin_sda, const uint8_t pin_scl, const u8g2_cb_t *rotation);
    void begin();
    void display(float weight, unsigned long time);
    void promptText(const char *prompt, const char *text);
    void centerText(const char *text, const uint8_t size);
    void switcher(const char *current, const uint8_t index, const uint8_t count, const char* options);
    void recipeSummary(const char *name, const char *description);
    void recipeCoffeeWeightConfig(const char *header, unsigned int weightMg, unsigned int waterWeightMl);
    void recipePour(uint32_t weightToPour, uint64_t timeToFinish, bool isPause, uint8_t pourIndex, uint8_t pours);
    void text(const char *text);
    void update(){};
    void clear();

private:
    U8G2_SH1107_64X128_F_HW_I2C u8g;
};

#endif