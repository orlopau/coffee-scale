#ifndef NATIVE

#pragma once

#include <RotaryEncoder.h>
#include "user_interface.h"

class EmbeddedUserInput : public UserInput
{
public:
    EmbeddedUserInput(uint8_t pin_enc_a, uint8_t pin_enc_b, uint8_t pin_enc_btn);
    void update();
    EncoderDirection getEncoderDirection();
    long getEncoderTicks();
    void resetEncoderTicks();
    ClickType getEncoderClick();

private:
    uint8_t encoderButtonPin;
    Button encoderButton;
    RotaryEncoder encoder;
};

#endif