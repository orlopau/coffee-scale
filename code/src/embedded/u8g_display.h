#ifndef NATIVE

#pragma once

#include <U8g2lib.h>

#include "user_interface.h"

class U8GDisplay : public Display
{
public:
    U8GDisplay(uint8_t pin_sda, uint8_t pin_scl);
    void begin();
    void display(float weight, unsigned long time);
    void update() {};
    void clear();
private:
    U8G2_SH1107_64X128_F_SW_I2C u8g;
};


#endif