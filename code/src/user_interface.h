#pragma once
#include <cstdio>
#include <math.h>
#include "button.h"

class UserInput
{
public:
    virtual ~UserInput() {}
    virtual void update() = 0;
    virtual int getEncoderTicks() = 0;
    virtual void resetEncoderTicks() = 0;
    virtual ClickType getEncoderClick() = 0;
    virtual ClickType getBootClick() = 0;
};

class Display
{
public:
    virtual ~Display() {}
    virtual void begin() = 0;
    virtual void display(float weight, unsigned long time) = 0;
    virtual void promptText(const char *prompt, const char *subtext) = 0;
    virtual void singleText(const char *text) = 0;
    virtual void update() = 0;
    virtual void clear() = 0;
};
