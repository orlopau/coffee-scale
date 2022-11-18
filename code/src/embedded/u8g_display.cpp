#ifndef NATIVE

#include "u8g_display.h"

U8GDisplay::U8GDisplay(uint8_t pin_sda, uint8_t pin_scl)
    : u8g(U8G2_R0, pin_scl, pin_sda, U8X8_PIN_NONE)
{
}

void U8GDisplay::begin()
{
    u8g.begin();
}

void U8GDisplay::clear()
{
    u8g.clearBuffer();
}

void U8GDisplay::display(float weight, unsigned long time)
{
    char *weightText = formatWeight(weight);
    char *timeText = formatTime(time);
    u8g.clearBuffer();
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(0, 10, weightText);
    u8g.drawStr(0, 20, timeText);
    u8g.sendBuffer();
}

#endif