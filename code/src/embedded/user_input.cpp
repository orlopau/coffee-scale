#ifndef NATIVE

#include "user_input.h"

EmbeddedUserInput::EmbeddedUserInput(uint8_t pin_enc_a, uint8_t pin_enc_b, uint8_t pin_enc_btn)
    : encoder(pin_enc_a, pin_enc_b, RotaryEncoder::LatchMode::FOUR3), encoderButton(), encoderButtonPin(pin_enc_btn)
{
    pinMode(pin_enc_btn, INPUT_PULLUP);
}

void EmbeddedUserInput::update()
{
    encoder.tick();
    encoderButton.update(digitalRead(encoderButtonPin) == LOW);
}

EncoderDirection EmbeddedUserInput::getEncoderDirection()
{
    switch (encoder.getDirection())
    {
    case RotaryEncoder::Direction::NOROTATION:
        return EncoderDirection::NONE;
    case RotaryEncoder::Direction::CLOCKWISE:
        return EncoderDirection::CW;
    case RotaryEncoder::Direction::COUNTERCLOCKWISE:
        return EncoderDirection::CCW;
    default:
        return EncoderDirection::NONE;
    }
}

long EmbeddedUserInput::getEncoderTicks()
{
    return encoder.getPosition();
}

void EmbeddedUserInput::setEncoderTicks(long ticks)
{
    encoder.setPosition(ticks);
}

void EmbeddedUserInput::resetEncoderTicks()
{
    encoder.setPosition(0);
}

ClickType EmbeddedUserInput::consumeEncoderClick()
{
    return encoderButton.consumeClickType();
}

ClickType EmbeddedUserInput::getEncoderClick()
{
    return encoderButton.getClickType();
}

bool EmbeddedUserInput::isEncoderPressed()
{
    return encoderButton.isPressed();
}

#endif