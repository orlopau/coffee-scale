#pragma once

#include "button.h"

namespace Interface
{
    void begin();
    /** Once per loop: takes the turns and the next click for this loop. */
    void update();
    /** Detents turned since the last update(), positive clockwise. The same for every caller in a loop. */
    int getEncoderTurns();
    long getEncoderTicks();
    void setEncoderTicks(long ticks);
    void resetEncoderTicks();
    /** Returns the click of this loop and clears it for the rest of the loop. */
    ClickType consumeEncoderClick();
    /** The click of this loop, NONE if there is none. */
    ClickType getEncoderClick();
    /** Clears the click of this loop and all queued clicks. */
    void discardClicks();
    bool isEncoderPressed();
    void buzzerTone(unsigned int durationMs);
}
