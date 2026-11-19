#include <unity.h>

#include "grinder/reconnect_backoff.h"

void setUp(void) {}
void tearDown(void) {}

void test_wait_doubles_up_to_the_limit(void)
{
    uint32_t wait = ReconnectBackoff::FIRST_MS;
    const uint32_t expected[] = {4000, 8000, 16000, 32000, 60000, 60000};
    for (uint32_t want : expected)
    {
        wait = ReconnectBackoff::next(wait);
        TEST_ASSERT_EQUAL_UINT32(want, wait);
    }
}

void test_wait_is_over_after_the_time(void)
{
    TEST_ASSERT_FALSE(ReconnectBackoff::over(1000, 4000, 4999));
    TEST_ASSERT_TRUE(ReconnectBackoff::over(1000, 4000, 5000));
}

void test_wait_across_the_clock_wrap(void)
{
    const uint32_t failedAt = 0xFFFFFFFFu - 999;
    TEST_ASSERT_FALSE(ReconnectBackoff::over(failedAt, 4000, 2000));
    TEST_ASSERT_TRUE(ReconnectBackoff::over(failedAt, 4000, 3000));
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_wait_doubles_up_to_the_limit);
    RUN_TEST(test_wait_is_over_after_the_time);
    RUN_TEST(test_wait_across_the_clock_wrap);
    return UNITY_END();
}
