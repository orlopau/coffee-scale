#pragma once

#include "interface.h"

namespace Interface
{
    extern ClickType encoderClick;
    extern ClickType bootClick;
    extern int encoderTicks;
    extern int encoderTurns;

    void reset();
}
