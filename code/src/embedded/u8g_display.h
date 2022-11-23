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
    void text(const char *text);
    void update() {};
    void clear();
private:
    U8G2_SH1107_64X128_F_HW_I2C u8g;
};


#endif