#ifndef NATIVE

#include <cstring>

#include "u8g_display.h"
#include "formatters.h"

U8GDisplay::U8GDisplay(const uint8_t pin_sda, const uint8_t pin_scl, const u8g2_cb_t *rotation)
    : u8g(rotation, U8X8_PIN_NONE, pin_scl, pin_sda)
{
}

void U8GDisplay::begin()
{
    u8g.setBusClock(1000000);
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
    u8g.setFont(u8g2_font_logisoso30_tf);
    u8g.drawStr(0, 30, weightText);
    u8g.setFont(u8g2_font_logisoso22_tf);
    u8g.drawStr(0, 64, timeText);
    u8g.sendBuffer();
}

void U8GDisplay::promptText(const char *prompt, const char *text)
{
    u8g.clearBuffer();
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(0, 10, prompt);
    u8g.drawStr(0, 20, text);
    u8g.sendBuffer();
}

void U8GDisplay::centerText(const char *text, const uint8_t size)
{
    u8g.clearBuffer();

    int mid = 0;
    switch (size)
    {
    case 16:
        u8g.setFont(u8g2_font_logisoso16_tf);
        mid = 32 + 8;
        break;
    case 30:
        u8g.setFont(u8g2_font_logisoso30_tf);
        mid = 32 + 15;
        break;
    default:
        u8g.setFont(u8g_font_6x10);
        mid = 32 + 5;
        break;
    }

    u8g.drawStr(0, mid, text);
    u8g.sendBuffer();
}

void U8GDisplay::switcher(const char *current, const uint8_t index, const uint8_t count, const char *options)
{
    u8g.clearBuffer();
    u8g.userInterfaceSelectionList("Select a recipe.", index, options);
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(0, 10, current);
    u8g.sendBuffer();
};

void U8GDisplay::recipeSummary(const char *name, const char *description) {}
void U8GDisplay::recipeCoffeeWeightConfig(const char *header, unsigned int weightMg, unsigned int waterWeightMl) {}
void U8GDisplay::recipePour(uint32_t weightToPour, uint64_t timeToFinish, bool isPause, uint8_t pourIndex, uint8_t pours) {}

void U8GDisplay::text(const char *text)
{
    u8g.clearBuffer();
    u8g.setFont(u8g_font_6x10);

    // split string at newline
    char *textCopy = strdup(text);
    char *pointer = strtok(textCopy, "\n");
    int line = 10;

    while (pointer != NULL)
    {
        u8g.drawStr(0, line, pointer);
        line += 10;
        pointer = strtok(NULL, "\n");
    }
    delete textCopy;

    u8g.sendBuffer();
}

#endif