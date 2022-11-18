#pragma once
#include <cstdio>
#include <math.h>

enum class ClickType {NONE, SINGLE, DOUBLE, LONG};

class Buttons
{
public:
    virtual ~Buttons() {}
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
    virtual void update() = 0;
    virtual void clear() = 0;
};

char *formatWeight(float weight);
char *formatTime(unsigned long time);

