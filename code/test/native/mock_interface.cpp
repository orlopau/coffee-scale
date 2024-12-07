#include "mock/mock_interface.h"

namespace Interface
{
    ClickType encoderClick = ClickType::NONE;
    ClickType bootClick = ClickType::NONE;
    int encoderTicks = 0;
    EncoderDirection encoderDirection = EncoderDirection::NONE;

    void reset()
    {
        encoderClick = ClickType::NONE;
        bootClick = ClickType::NONE;
        encoderTicks = 0;
        encoderDirection = EncoderDirection::NONE;
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
    EncoderDirection getEncoderDirection() { return encoderDirection; }
    void setEncoderTicks(long ticks) { encoderTicks = ticks; }
    ClickType getBootClick() { return bootClick; }
    void buzzerTone(unsigned int durationMs) {}
}