#pragma once

#include "interface.h"

namespace Interface
{
    extern ClickType encoderClick;
    extern ClickType bootClick;
    extern int encoderTicks;
    extern int encoderTurns;
    /** Beeps of the last buzzerTone() or buzzerPattern() since reset(), 0 if none. */
    extern unsigned int buzzerBeeps;

    void reset();
}
