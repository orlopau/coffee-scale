#pragma once

#include <cstdint>

/**
 * @brief Enum for button click types.
 */
enum class ClickType
{
    NONE,
    SINGLE,
    LONG
};

/**
 * @brief Debounced button detecting single, double and long clicks.
 */
class Button
{
public:
    /**
     * @brief Construct a new Button object.
     *
     * @param debounceDelay debounce delay in ms
     * @param doubleClickWaitTime maximum time between clicks to be considered a double click in ms
     * @param longClickDelay long click delay in ms
     */
    Button(unsigned long debounceDelay = 50, unsigned long doubleClickWaitTime = 200, unsigned long longClickDelay = 1000);

    /**
     * @brief Update the button state, must be called regularly or via an interrupt.
     *
     * @param isPressed true if the button is currently pressed
     */
    void update(bool isPressed);

    /**
     * @brief Get the current click type.
     */
    ClickType getClickType();

    /**
     * @brief Returns the debounced button state.
     *
     * @return true when pressed, false when released
     */
    bool isPressed();

private:
    unsigned long debounceDelay;
    unsigned long doubleClickWaitTime;
    unsigned long longClickDelay;

    unsigned long lastPressChangeTime = 0;
    bool lastButtonState = false;
    bool buttonStateDebounced = false;

    void updateClickType(bool isPressed);
    enum States
    {
        STATE_LOW,
        RISING_FLANK,
        STATE_HIGH,
        FALLING_FLANK,
    };
    States state = STATE_LOW;
    unsigned long lastPressTime = 0;
    unsigned long lastReleaseTime = 0;
    ClickType clickType = ClickType::NONE;
};