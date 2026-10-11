#include "mock/mock_interface.h"

namespace Interface
{
    ClickType encoderClick = ClickType::NONE;
    ClickType bootClick = ClickType::NONE;
    int encoderTicks = 0;
    int encoderTurns = 0;

    void reset()
    {
        encoderClick = ClickType::NONE;
        bootClick = ClickType::NONE;
        encoderTicks = 0;
        encoderTurns = 0;
    }

    void update() {}
    long getEncoderTicks() { return encoderTicks; }
    void resetEncoderTicks() { encoderTicks = 0; }
    ClickType consumeEncoderClick()
    {
        ClickType click = encoderClick;
        encoderClick = ClickType::NONE;
        return click;
    }
    ClickType getEncoderClick() { return encoderClick; }
    void discardClicks() { encoderClick = ClickType::NONE; }
    int getEncoderTurns() { return encoderTurns; }
    void setEncoderTicks(long ticks) { encoderTicks = ticks; }
    ClickType getBootClick() { return bootClick; }
    void buzzerTone(unsigned int durationMs) {}
}
