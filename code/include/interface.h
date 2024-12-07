#pragma once

#include "button.h"

namespace Interface
{
    enum class EncoderDirection
    {
        NONE = 0,
        CW = 1,
        CCW = -1,
    };

    void begin();
    void update();
    EncoderDirection getEncoderDirection();
    long getEncoderTicks();
    void setEncoderTicks(long ticks);
    void resetEncoderTicks();
    ClickType consumeEncoderClick();
    ClickType getEncoderClick();
    bool isEncoderPressed();
    void buzzerTone(unsigned int durationMs);
}