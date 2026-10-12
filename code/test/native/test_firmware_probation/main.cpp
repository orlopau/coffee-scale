#include <unity.h>

#include "firmware_probation.h"

void setUp(void) {}

void tearDown(void) {}

void test_not_before_two_seconds(void)
{
    FirmwareProbation probation;
    TEST_ASSERT_FALSE(probation.update(1000, true));
    TEST_ASSERT_FALSE(probation.update(2999, true));
    TEST_ASSERT_TRUE(probation.update(3000, true));
}

void test_only_once(void)
{
    FirmwareProbation probation;
    probation.update(0, true);
    TEST_ASSERT_TRUE(probation.update(2000, false));
    TEST_ASSERT_FALSE(probation.update(4000, true));
}

// readings in any loop count, not only in the last one
void test_readings_from_an_earlier_loop_count(void)
{
    FirmwareProbation probation;
    probation.update(0, false);
    probation.update(100, true);
    TEST_ASSERT_TRUE(probation.update(2000, false));
}

// e.g. a firmware whose load cell task doesn't run
void test_never_without_readings(void)
{
    FirmwareProbation probation;
    probation.update(0, false);
    TEST_ASSERT_FALSE(probation.update(60000, false));
    TEST_ASSERT_TRUE(probation.update(60100, true));
}

// the clock starts at the first loop, not at boot
void test_counts_from_the_first_loop(void)
{
    FirmwareProbation probation;
    TEST_ASSERT_FALSE(probation.update(5000, true));
    TEST_ASSERT_FALSE(probation.update(6999, true));
    TEST_ASSERT_TRUE(probation.update(7000, true));
}

void test_millis_wrapping_around(void)
{
    FirmwareProbation probation;
    probation.update(0xFFFFFC00u, true);
    TEST_ASSERT_FALSE(probation.update(0x00000100u, true));
    TEST_ASSERT_TRUE(probation.update(0xFFFFFC00u + FirmwareProbation::RUN_MS, true));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_not_before_two_seconds);
    RUN_TEST(test_only_once);
    RUN_TEST(test_readings_from_an_earlier_loop_count);
    RUN_TEST(test_never_without_readings);
    RUN_TEST(test_counts_from_the_first_loop);
    RUN_TEST(test_millis_wrapping_around);
    return UNITY_END();
}
