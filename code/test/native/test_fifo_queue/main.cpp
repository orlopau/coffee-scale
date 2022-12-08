#include <unity.h>
#include "ring_buffer.h"

/*
0 1 2

1 2 3
4
*/

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

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_ring_buffer_push);
    RUN_TEST(test_ring_buffer_get);
    RUN_TEST(test_ring_buffer_get_relative);
    RUN_TEST(test_ring_buffer_sum_last);
    UNITY_END();
}