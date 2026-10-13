#include <unity.h>

#include "auto_update_policy.h"

void setUp(void) {}

void tearDown(void) {}

void test_counts_starts_up_to_255(void)
{
    TEST_ASSERT_EQUAL(1, AutoUpdatePolicy::countStart(0));
    TEST_ASSERT_EQUAL(5, AutoUpdatePolicy::countStart(4));
    TEST_ASSERT_EQUAL(255, AutoUpdatePolicy::countStart(254));
    TEST_ASSERT_EQUAL(255, AutoUpdatePolicy::countStart(255));
}

void test_due_from_the_fifth_start(void)
{
    TEST_ASSERT_FALSE(AutoUpdatePolicy::isDue(0, false));
    TEST_ASSERT_FALSE(AutoUpdatePolicy::isDue(4, false));
    TEST_ASSERT_TRUE(AutoUpdatePolicy::isDue(5, false));
    TEST_ASSERT_TRUE(AutoUpdatePolicy::isDue(255, false));
}

// a scale that never checked: AutoUpdate reads CHECK_EVERY_STARTS - 1 when nothing is stored
void test_a_scale_that_never_checked_checks_at_once(void)
{
    TEST_ASSERT_TRUE(AutoUpdatePolicy::isDue(AutoUpdatePolicy::countStart(AutoUpdatePolicy::CHECK_EVERY_STARTS - 1),
                                             false));
}

// a crash during the check would roll back a good update
void test_never_due_on_probation(void)
{
    TEST_ASSERT_FALSE(AutoUpdatePolicy::isDue(5, true));
    TEST_ASSERT_FALSE(AutoUpdatePolicy::isDue(255, true));
}

void test_offers_only_newer_versions(void)
{
    TEST_ASSERT_TRUE(AutoUpdatePolicy::worthOffering("v1.10.0", "v1.9.0", ""));
    TEST_ASSERT_FALSE(AutoUpdatePolicy::worthOffering("v1.9.0", "v1.9.0", ""));
    TEST_ASSERT_FALSE(AutoUpdatePolicy::worthOffering("v1.8.0", "v1.9.0", ""));
    TEST_ASSERT_FALSE(AutoUpdatePolicy::worthOffering("", "v1.9.0", ""));
}

void test_never_offers_the_version_rolled_back(void)
{
    TEST_ASSERT_FALSE(AutoUpdatePolicy::worthOffering("v1.10.0", "v1.9.0", "v1.10.0"));
    // a newer release after the broken one
    TEST_ASSERT_TRUE(AutoUpdatePolicy::worthOffering("v1.10.1", "v1.9.0", "v1.10.0"));
}

void test_asks_about_what_is_worth_offering(void)
{
    TEST_ASSERT_TRUE(AutoUpdatePolicy::shouldAsk("v1.10.0", "v1.9.0", "", false));
    TEST_ASSERT_FALSE(AutoUpdatePolicy::shouldAsk("v1.10.0", "v1.9.0", "v1.10.0", false));
    // installed meanwhile, e.g. with the button at boot
    TEST_ASSERT_FALSE(AutoUpdatePolicy::shouldAsk("v1.10.0", "v1.10.0", "", false));
    TEST_ASSERT_FALSE(AutoUpdatePolicy::shouldAsk("", "v1.9.0", "", false));
}

// switching off at the question would roll back a good firmware
void test_never_asks_on_probation(void)
{
    TEST_ASSERT_FALSE(AutoUpdatePolicy::shouldAsk("v1.10.0", "v1.9.0", "", true));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_counts_starts_up_to_255);
    RUN_TEST(test_due_from_the_fifth_start);
    RUN_TEST(test_a_scale_that_never_checked_checks_at_once);
    RUN_TEST(test_never_due_on_probation);
    RUN_TEST(test_offers_only_newer_versions);
    RUN_TEST(test_never_offers_the_version_rolled_back);
    RUN_TEST(test_asks_about_what_is_worth_offering);
    RUN_TEST(test_never_asks_on_probation);
    return UNITY_END();
}
