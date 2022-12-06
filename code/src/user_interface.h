#pragma once
#include <cstdio>
#include <math.h>
#include "button.h"

enum class EncoderDirection
{
    NONE = 0,
    CW = 1,
    CCW = -1,
};

class UserInput
{
public:
    virtual ~UserInput() {}
    virtual void update() = 0;
    virtual EncoderDirection getEncoderDirection() = 0;
    virtual long getEncoderTicks() = 0;
    virtual void resetEncoderTicks() = 0;
    virtual void setEncoderTicks(long ticks) = 0;
    virtual ClickType getEncoderClick() = 0;
    virtual ClickType consumeEncoderClick() = 0;
};

class Display
{
public:
    virtual ~Display() {}
    virtual void begin() = 0;
    virtual void display(float weight, unsigned long time) = 0;
    virtual void promptText(const char *prompt, const char *subtext) = 0;
    virtual void centerText(const char *text, const uint8_t size) = 0;
    virtual void modeSwitcher(const char *current, const uint8_t index, const uint8_t count, float batV, float batPercentage, bool batCharging) = 0;
    virtual void switcher(const char *current, const uint8_t index, const uint8_t count, const char *options[]) = 0;
    virtual void recipeSummary(const char *name, const char *description) = 0;
    virtual void recipeCoffeeWeightConfig(const char *header, unsigned int weightMg, unsigned int waterWeightMl) = 0;
    virtual void recipeConfigRatio(const char *header, float coffee, float water) = 0;
    virtual void recipePour(const char *text, uint32_t weightToPourMg, uint64_t timeToFinishMs, bool isPause, uint8_t pourIndex, uint8_t pours) = 0;
    virtual void text(const char *text) = 0;
    virtual void update() = 0;
    virtual void clear() = 0;
};
