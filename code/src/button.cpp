#include "util/millis.h"
#include "button.h"

Button::Button(unsigned long debounceDelay, unsigned long doubleClickWaitTime, unsigned long longClickDelay)
    : debounceDelay(debounceDelay), doubleClickWaitTime(doubleClickWaitTime), longClickDelay(longClickDelay) {}

void Button::update(bool isPressed)
{
    if (isPressed != lastButtonState)
    {
        lastPressChangeTime = now();
    }

    if (now() - lastPressChangeTime > debounceDelay)
    {
        // new debounced button state
        buttonStateDebounced = isPressed;
    }

    updateClickType(buttonStateDebounced);

    lastButtonState = isPressed;
}

void Button::updateClickType(bool isPressed)
{
    switch (state)
    {
    case States::STATE_LOW:
        // now pressed
        if (isPressed)
        {
            state = States::STATE_HIGH;
            lastPressTime = now();
        }
        // still not pressed
        else
        {
            clickType = ClickType::NONE;
        }
        break;
    case States::STATE_HIGH:
        // still pressed
        if (isPressed)
        {
            // when button is held longer than long click, and click type is not already long
            // click, register a long click.
            if (now() - lastPressTime > longClickDelay)
            {
                clickType = ClickType::LONG;
                // change state to longpress
                state = States::STATE_LONGPRESS;
            }
            else
            {
                clickType = ClickType::NONE;
            }
        }
        // press released
        else
        {
            state = States::STATE_LOW;
            if (now() - lastPressTime < longClickDelay)
            {
                clickType = ClickType::SINGLE;
            }
        }
        break;
    case States::STATE_LONGPRESS:
        // when in a longpress, do not register further longpresses
        clickType = ClickType::NONE;
        // exit longpress state when button is released
        if (!isPressed)
        {
            state = States::STATE_LOW;
        }
    }
}

bool Button::isPressed()
{
    return buttonStateDebounced;
}

ClickType Button::consumeClickType()
{
    ClickType result = clickType;
    clickType = ClickType::NONE;
    return result;
}

ClickType Button::getClickType()
{
    return clickType;
}