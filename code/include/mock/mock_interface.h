#pragma once

#include "interface.h"

namespace Interface
{
    extern ClickType encoderClick;
    extern ClickType bootClick;
    extern int encoderTicks;
    extern EncoderDirection encoderDirection;

    void reset();
}