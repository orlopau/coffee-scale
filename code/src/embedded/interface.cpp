#ifndef NATIVE

#include <Arduino.h>

#include "interface.h"
#include "constants.h"
#include "millis.h"
#include "embedded/button_task.h"
#include "embedded/click_queue.h"
#include "embedded/encoder_counter.h"
#include "input/click_inbox.h"
#include "input/detent_counter.h"

static EncoderCounter encoderCounter;
static DetentCounter detentCounter;
static ClickQueue clickQueue;
static ButtonTask buttonTask(clickQueue);
static ClickInbox clickInbox(clickQueue);

static int32_t lastDetent = 0;
static int turns = 0;
static long ticks = 0;

void Interface::begin()
{
    pinMode(PIN_ENC_BTN, INPUT_PULLUP);
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);

    encoderCounter.begin();
    // without the queue the task still runs, for the buzzer and isEncoderPressed()
    clickQueue.begin();
    buttonTask.begin();
}

void Interface::update()
{
    int32_t detent = detentCounter.update(encoderCounter.read());
    turns = detent - lastDetent;
    lastDetent = detent;
    ticks += turns;

    clickInbox.update(now());
}

int Interface::getEncoderTurns() { return turns; }

long Interface::getEncoderTicks() { return ticks; }

void Interface::setEncoderTicks(long value) { ticks = value; }

void Interface::resetEncoderTicks() { ticks = 0; }

ClickType Interface::consumeEncoderClick() { return clickInbox.consume(); }

ClickType Interface::getEncoderClick() { return clickInbox.get(); }

void Interface::discardClicks() { clickInbox.discard(); }

bool Interface::isEncoderPressed() { return buttonTask.isPressed(); }

void Interface::buzzerTone(unsigned int durationMs) { buttonTask.buzz(durationMs); }

#endif
