#ifndef NATIVE

#pragma once

#include <RotaryEncoder.h>
#include "user_interface.h"

class EmbeddedUserInput : public UserInput
{
public:
    EmbeddedUserInput(uint8_t pin_enc_a, uint8_t pin_enc_b, uint8_t pin_enc_btn, uint8_t pin_buzzer);
    void update();
    EncoderDirection getEncoderDirection();
    long getEncoderTicks();
    void setEncoderTicks(long ticks);
    void resetEncoderTicks();
    ClickType consumeEncoderClick();
    ClickType getEncoderClick();
    bool isEncoderPressed();
    void buzzerTone(uint16_t durationMs);

private:
    uint8_t encoderButtonPin;
    uint8_t buzzerPin;
    Button encoderButton;
    RotaryEncoder encoder;

    uint64_t lastBuzzerMillis;
    uint32_t lastBuzzerDuration;
};

#endif