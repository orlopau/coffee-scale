#include <unity.h>

#include "input/detent_counter.h"

DetentCounter counter;

void setUp(void) { counter = DetentCounter(); }

void tearDown(void) {}

void test_changes_detent_at_three_counts(void)
{
    TEST_ASSERT_EQUAL(0, counter.update(1));
    TEST_ASSERT_EQUAL(0, counter.update(2));
    TEST_ASSERT_EQUAL(1, counter.update(3));
    TEST_ASSERT_EQUAL(1, counter.update(4));
    TEST_ASSERT_EQUAL(1, counter.update(6));
    TEST_ASSERT_EQUAL(2, counter.update(7));
    TEST_ASSERT_EQUAL(2, counter.update(8));
}

void test_counts_backwards_across_zero(void)
{
    TEST_ASSERT_EQUAL(0, counter.update(-2));
    TEST_ASSERT_EQUAL(-1, counter.update(-3));
    TEST_ASSERT_EQUAL(-1, counter.update(-4));
    TEST_ASSERT_EQUAL(-2, counter.update(-7));
    TEST_ASSERT_EQUAL(-2, counter.update(-8));
}

void test_bounce_at_an_edge_keeps_the_detent(void)
{
    TEST_ASSERT_EQUAL(1, counter.update(3));
    for (int i = 0; i < 5; i++)
    {
        TEST_ASSERT_EQUAL(1, counter.update(2));
        TEST_ASSERT_EQUAL(1, counter.update(3));
    }
    TEST_ASSERT_EQUAL(1, counter.update(4));
}

void test_absorbs_a_lost_count(void)
{
    // one count got lost: the detents now rest at 3, 7, 11 instead of 4, 8, 12
    TEST_ASSERT_EQUAL(1, counter.update(3));
    TEST_ASSERT_EQUAL(2, counter.update(7));
    TEST_ASSERT_EQUAL(3, counter.update(11));
    TEST_ASSERT_EQUAL(2, counter.update(7));
    TEST_ASSERT_EQUAL(1, counter.update(3));
    TEST_ASSERT_EQUAL(0, counter.update(-1));
}

void test_counts_all_detents_of_a_jump(void)
{
    TEST_ASSERT_EQUAL(3, counter.update(12));
    TEST_ASSERT_EQUAL(-3, counter.update(-12));
}

void test_back_and_forth_between_reads(void)
{
    TEST_ASSERT_EQUAL(1, counter.update(4));
    // turned +2 and -2 detents between two reads
    TEST_ASSERT_EQUAL(1, counter.update(4));
    TEST_ASSERT_EQUAL(2, counter.update(8));
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_changes_detent_at_three_counts);
    RUN_TEST(test_counts_backwards_across_zero);
    RUN_TEST(test_bounce_at_an_edge_keeps_the_detent);
    RUN_TEST(test_absorbs_a_lost_count);
    RUN_TEST(test_counts_all_detents_of_a_jump);
    RUN_TEST(test_back_and_forth_between_reads);
    return UNITY_END();
}
