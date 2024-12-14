#include "ring_buffer.h"
#include <unity.h>

/*
0 1 2

1 2 3
4
*/

void setUp(void) {}

void tearDown(void) {}

void test_ring_buffer_push(void)
{
    RingBuffer<int> ringBuffer(3);
    TEST_ASSERT_EQUAL(3, ringBuffer.capacity());
    TEST_ASSERT_EQUAL(0, ringBuffer.size());

    ringBuffer.push(1);
    TEST_ASSERT_EQUAL(1, ringBuffer.size());
    ringBuffer.push(2);
    TEST_ASSERT_EQUAL(2, ringBuffer.size());
    ringBuffer.push(3);
    TEST_ASSERT_EQUAL(3, ringBuffer.size());
    ringBuffer.push(4);
    TEST_ASSERT_EQUAL(3, ringBuffer.size());
}

void test_ring_buffer_get(void)
{
    RingBuffer<int> ringBuffer(3);
    // push 4 elements, last 3 should remain
    ringBuffer.push(1);
    ringBuffer.push(2);
    ringBuffer.push(3);
    ringBuffer.push(4);

    /*
    0 1 2

    1 2 3
    4
    */

    TEST_ASSERT_EQUAL(4, ringBuffer.get(0));
    TEST_ASSERT_EQUAL(2, ringBuffer.get(1));
    TEST_ASSERT_EQUAL(3, ringBuffer.get(2));
}

void test_ring_buffer_get_relative(void)
{
    RingBuffer<int> ringBuffer(3);
    ringBuffer.push(1);
    ringBuffer.push(2);
    ringBuffer.push(3);

    TEST_ASSERT_EQUAL(3, ringBuffer.getRelative(0));
    TEST_ASSERT_EQUAL(2, ringBuffer.getRelative(-1));
    TEST_ASSERT_EQUAL(1, ringBuffer.getRelative(-2));
}

void test_ring_buffer_sum_last(void)
{
    RingBuffer<int> ringBuffer(3);
    ringBuffer.push(1);
    ringBuffer.push(2);
    ringBuffer.push(3);

    TEST_ASSERT_EQUAL(3, ringBuffer.sumLast(1));
    TEST_ASSERT_EQUAL(5, ringBuffer.sumLast(2));
    TEST_ASSERT_EQUAL(6, ringBuffer.sumLast(3));
}

void test_ring_buffer_variance_and_stddev(void)
{
    RingBuffer<float> ringBuffer(5);
    ringBuffer.push(1.0);
    ringBuffer.push(2.0);
    ringBuffer.push(3.0);
    ringBuffer.push(4.0);
    ringBuffer.push(5.0);

    // variance for 5 values: [1, 2, 3, 4, 5]
    float variance = ringBuffer.varianceLast(5);
    float expectedVariance = 2.0f;                              // ( (1-3)^2 + (2-3)^2 + (3-3)^2 + (4-3)^2 + (5-3)^2 ) / 5 = 2
    TEST_ASSERT_FLOAT_WITHIN(0.01, expectedVariance, variance); // allow small margin of error

    // standard deviation is the square root of the variance
    float stddev = ringBuffer.standardDeviationLast(5);
    float expectedStddev = std::sqrt(expectedVariance);     // sqrt(2) ≈ 1.414
    TEST_ASSERT_FLOAT_WITHIN(0.01, expectedStddev, stddev); // allow small margin of error
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_ring_buffer_push);
    RUN_TEST(test_ring_buffer_get);
    RUN_TEST(test_ring_buffer_get_relative);
    RUN_TEST(test_ring_buffer_sum_last);
    RUN_TEST(test_ring_buffer_variance_and_stddev);
    UNITY_END();
}