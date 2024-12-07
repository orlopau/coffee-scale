#ifndef NATIVE

#include <Arduino.h>
#include <RotaryEncoder.h>
#include "interface.h"
#include "constants.h"
#include "button.h"

static RotaryEncoder encoder(PIN_ENC_A, PIN_ENC_B, RotaryEncoder::LatchMode::FOUR3);
static Button encoderButton;
static uint64_t lastBuzzerMillis;
static uint32_t lastBuzzerDuration;


void Interface::begin() {
    pinMode(PIN_ENC_BTN, INPUT_PULLUP);
    pinMode(PIN_BUZZER, OUTPUT);
}

void Interface::update()
{
    encoder.tick();
    encoderButton.update(digitalRead(PIN_ENC_BTN) == LOW);

    if (lastBuzzerMillis + lastBuzzerDuration < millis())
    {
        digitalWrite(PIN_BUZZER, LOW);
    }
}

Interface::EncoderDirection Interface::getEncoderDirection()
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

long Interface::getEncoderTicks()
{
    return encoder.getPosition();
}

void Interface::setEncoderTicks(long ticks)
{
    encoder.setPosition(ticks);
}

void Interface::resetEncoderTicks()
{
    encoder.setPosition(0);
}

ClickType Interface::consumeEncoderClick()
{
    return encoderButton.consumeClickType();
}

ClickType Interface::getEncoderClick()
{
    return encoderButton.getClickType();
}

bool Interface::isEncoderPressed()
{
    return encoderButton.isPressed();
}

void Interface::buzzerTone(unsigned int durationMs)
{
    digitalWrite(PIN_BUZZER, HIGH);
    lastBuzzerMillis = millis();
    lastBuzzerDuration = durationMs;
}

#endif