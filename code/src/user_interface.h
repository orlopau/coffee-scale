#pragma once
#include <cstdio>
#include <math.h>
#include "button.h"

enum class EncoderDirection
{
    NONE,
    CW,
    CCW,
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
};

class Display
{
public:
    virtual ~Display() {}
    virtual void begin() = 0;
    virtual void display(float weight, unsigned long time) = 0;
    virtual void promptText(const char *prompt, const char *subtext) = 0;
    virtual void text(const char *text) = 0;
    virtual void update() = 0;
    virtual void clear() = 0;
};
