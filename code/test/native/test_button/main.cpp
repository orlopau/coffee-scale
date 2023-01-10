#include <unity.h>

#include "button.h"
#include "millis.h"

#define DEBOUNCE_DELAY 50
#define DOUBLE_CLICK_WAIT_TIME 200
#define LONG_CLICK_DELAY 1000

Button button;

void debounced_press(bool isPressed)
{
    button.update(isPressed);
    sleep_for(DEBOUNCE_DELAY + 1);
    button.update(isPressed);
}

void test_debounce()
{
    TEST_ASSERT_FALSE(button.isPressed());
    // button is pushed for half the debounce delay
    button.update(true);
    TEST_ASSERT_FALSE(button.isPressed());
    sleep_for(DEBOUNCE_DELAY / 2.0);
    // debounce delay not reached yet
    button.update(true);
    TEST_ASSERT_FALSE(button.isPressed());
    // when pushed longer it is registered as a press
    sleep_for(DEBOUNCE_DELAY / 2 + 1);
    button.update(true);
    TEST_ASSERT_TRUE(button.isPressed());

    // same for release
    button.update(false);
    sleep_for(DEBOUNCE_DELAY / 2);
    TEST_ASSERT_TRUE(button.isPressed());
    sleep_for(DEBOUNCE_DELAY / 2 + 1);
    button.update(false);
    TEST_ASSERT_FALSE(button.isPressed());
}

void test_debounce_timer_reset()
{
    // when the button press state changed again before the debounce delay is reached,
    // the debounce timer is reset and the button state is not changed
    TEST_ASSERT_FALSE(button.isPressed());
    button.update(true);
    sleep_for(DEBOUNCE_DELAY / 2);
    button.update(false);
    TEST_ASSERT_FALSE(button.isPressed());
    sleep_for(DEBOUNCE_DELAY / 2 + 1);
    button.update(false);
    // although we waited for more than the debounce delay, the button press state did not change
    TEST_ASSERT_FALSE(button.isPressed());
}

void test_single_click(void)
{
    TEST_ASSERT_EQUAL(ClickType::NONE, button.getClickType());
    // click button for longer than debounce time
    debounced_press(true);

    // no click yet
    TEST_ASSERT_EQUAL(ClickType::NONE, button.getClickType());

    // release button
    debounced_press(false);

    // single click
    TEST_ASSERT_EQUAL(ClickType::SINGLE, button.getClickType());
}

void test_long_click(void)
{
    debounced_press(true);
    // after the delay, a long click is registered
    sleep_for(LONG_CLICK_DELAY + 10);
    button.update(true);
    TEST_ASSERT_EQUAL(ClickType::LONG, button.getClickType());

    // but only for a single tick, even when button is still pressed
    button.update(true);
    TEST_ASSERT_EQUAL(ClickType::NONE, button.getClickType());

    // even after further ticks
    button.update(true);
    TEST_ASSERT_EQUAL(ClickType::NONE, button.getClickType());
}

void test_click_is_reset_with_update_pressed(void)
{
    // click button for longer than long click delay
    debounced_press(true);
    debounced_press(false);

    // update again, should reset to none
    button.update(true);
    TEST_ASSERT_EQUAL(ClickType::NONE, button.getClickType());
}

void test_click_is_reset_with_update_released(void)
{
    // click button for longer than long click delay
    button.update(true);
    sleep_for(LONG_CLICK_DELAY + 10);
    button.update(true);

    // update again, should reset to none
    button.update(false);
    TEST_ASSERT_EQUAL(ClickType::NONE, button.getClickType());
}

void setUp(void) { button = Button(DEBOUNCE_DELAY, DOUBLE_CLICK_WAIT_TIME, LONG_CLICK_DELAY); }

void tearDown(void) {}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_debounce);
    RUN_TEST(test_debounce_timer_reset);

    RUN_TEST(test_single_click);
    RUN_TEST(test_long_click);
    RUN_TEST(test_click_is_reset_with_update_pressed);
    RUN_TEST(test_click_is_reset_with_update_released);
    UNITY_END();
}