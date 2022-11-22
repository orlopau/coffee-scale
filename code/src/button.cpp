#include "millis.h"
#include "button.h"

Button::Button(unsigned long debounceDelay, unsigned long doubleClickWaitTime, unsigned long longClickDelay)
    : debounceDelay(debounceDelay), doubleClickWaitTime(doubleClickWaitTime), longClickDelay(longClickDelay) {}

void Button::update(bool isPressed)
{
    // reset click type on every tick
    clickType = ClickType::NONE;

    if (isPressed != lastButtonState)
    {
        lastPressChangeTime = now();
    }

    if (now() - lastPressChangeTime > debounceDelay)
    {
        // new debounced button state
        buttonStateDebounced = isPressed;
        updateClickType(buttonStateDebounced);
    }

    lastButtonState = isPressed;
}

void Button::updateClickType(bool isPressed)
{
    switch (state)
    {
    case States::STATE_LOW:
        if (isPressed)
        {
            state = States::STATE_HIGH;
            lastPressTime = now();
        }
        else
        {
            clickType = ClickType::NONE;
        }
        break;
    case States::STATE_HIGH:
        if (!isPressed)
        {
            state = States::STATE_LOW;
            if (now() - lastPressTime < longClickDelay)
            {
                clickType = ClickType::SINGLE;
            }
            else
            {
                clickType = ClickType::LONG;
            }
        }
        else
        {
            clickType = ClickType::NONE;
        }
        break;
    }
}

bool Button::isPressed()
{
    return buttonStateDebounced;
}

ClickType Button::getClickType()
{
    return clickType;
}