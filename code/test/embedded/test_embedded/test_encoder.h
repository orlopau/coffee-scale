#include <unity.h>

#include "embedded/user_input.h"
#include "embedded/constants.h"
#include "confirm.h"

void test_encoder_direction(void)
{
    EmbeddedUserInput input(PIN_ENC_A, PIN_ENC_B, PIN_ENC_BTN);
    TEST_ASSERT_EQUAL(EncoderDirection::NONE, input.getEncoderDirection());
    Serial.println("Turn encoder clockwise");
    while (input.getEncoderDirection() != EncoderDirection::CW)
    {
        input.update();
    }
    Serial.println("Turn encoder counter-clockwise");
    while (input.getEncoderDirection() != EncoderDirection::CCW)
    {
        input.update();
    }
}

void test_encoder_ticks(void)
{
    EmbeddedUserInput input(PIN_ENC_A, PIN_ENC_B, PIN_ENC_BTN);
    TEST_ASSERT_EQUAL(0, input.getEncoderTicks());
    Serial.println("Turn encoder 3 ticks clockwise");
    while (input.getEncoderTicks() < 3)
    {
        input.update();
    }
    TEST_ASSERT_EQUAL(3, input.getEncoderTicks());
    Serial.println("Turn encoder 6 ticks counter-clockwise");
    while (input.getEncoderTicks() > -3)
    {
        input.update();
    }
    TEST_ASSERT_EQUAL(-3, input.getEncoderTicks());
}

void test_encoder_click(void)
{
    EmbeddedUserInput input(PIN_ENC_A, PIN_ENC_B, PIN_ENC_BTN);

}

void test_encoder_all()
{
    RUN_TEST(test_encoder_direction);
    RUN_TEST(test_encoder_ticks);
    RUN_TEST(test_encoder_click);
}