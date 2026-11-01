#include <unity.h>

#include "shot_curve.h"

static ShotCurve curve;

void setUp(void) { curve.reset(); }

void tearDown(void) {}

/** Readings at fromMs, fromMs + stepMs, … up to toMs, gramsPerSecond since t = 0. */
static void feed(ShotCurve &curve, uint32_t fromMs, uint32_t toMs, uint32_t stepMs, float gramsPerSecond)
{
    for (uint32_t t = fromMs; t <= toMs; t += stepMs)
    {
        curve.add(t, gramsPerSecond * t / 1000);
    }
}

void test_points_come_every_200_ms(void)
{
    feed(curve, 100, 6000, 100, 2);
    curve.finish(6000);
    TEST_ASSERT_EQUAL(31, curve.size());
    TEST_ASSERT_EQUAL_FLOAT(0, curve.gramsAt(0));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.4f, curve.gramsAt(1));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 12.0f, curve.gramsAt(30));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 12.0f, curve.yieldGrams());
    TEST_ASSERT_TRUE(curve.isFinished());
}

void test_a_point_takes_the_newest_reading_at_or_before_it(void)
{
    curve.add(150, 1);
    curve.add(390, 2);
    curve.add(400, 3);
    curve.add(450, 4);
    curve.finish(600);
    TEST_ASSERT_EQUAL(4, curve.size());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1, curve.gramsAt(1));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 3, curve.gramsAt(2));
}

// a detected shot starts at the first drop minus the offset, flat until the drop
void test_flat_until_the_first_reading(void)
{
    for (uint32_t t = 7000; t <= 12000; t += 100)
    {
        curve.add(t, 3);
    }
    curve.finish(12000);
    for (size_t i = 0; i < 35; i++)
    {
        TEST_ASSERT_EQUAL_FLOAT(0, curve.gramsAt(i));
    }
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 3, curve.gramsAt(35));
}

// the pump stop is reported after the readings past it went in
void test_points_after_the_end_are_dropped(void)
{
    feed(curve, 100, 12000, 100, 1);
    curve.finish(10000);
    TEST_ASSERT_EQUAL(51, curve.size());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.0f, curve.yieldGrams());
    TEST_ASSERT_EQUAL(10000, curve.durationMs());
}

// a click ends the shot after the last reading
void test_the_end_fills_up_to_it(void)
{
    feed(curve, 100, 9900, 100, 1);
    curve.finish(10000);
    TEST_ASSERT_EQUAL(51, curve.size());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 9.9f, curve.gramsAt(50));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 9.9f, curve.yieldGrams());
}

// its time goes on, and its last point is still the weight at the end
void test_the_curve_stops_at_90_s(void)
{
    // the pump stop noticed a second late
    feed(curve, 100, 96000, 100, 1);
    curve.finish(95000);
    TEST_ASSERT_EQUAL(450, curve.size());
    TEST_ASSERT_EQUAL(95000, curve.durationMs());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 89.6f, curve.gramsAt(448));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 95.0f, curve.yieldGrams());
    TEST_ASSERT_TRUE(curve.isFinished());
}

// readings between the points: the target's reading at 18050 ms, the stop at 18060
void test_the_last_point_is_the_newest_reading_at_or_before_the_end(void)
{
    feed(curve, 50, 18050, 100, 2);
    curve.finish(18060);
    TEST_ASSERT_EQUAL(91, curve.size());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 36.1f, curve.yieldGrams());
}

// a pump stop reported after the readings past it came in
void test_the_last_point_is_the_weight_at_a_late_end(void)
{
    feed(curve, 50, 19550, 100, 2);
    curve.finish(18060);
    TEST_ASSERT_EQUAL(91, curve.size());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 36.1f, curve.yieldGrams());
}

void test_short_or_light_shots_are_not_finished(void)
{
    feed(curve, 100, 4900, 100, 2);
    curve.finish(4900);
    TEST_ASSERT_FALSE(curve.isFinished());

    curve.reset();
    feed(curve, 100, 6000, 100, 0.1f);
    curve.finish(6000);
    TEST_ASSERT_FALSE(curve.isFinished());

    curve.reset();
    feed(curve, 100, 5000, 100, 2);
    curve.finish(5000);
    TEST_ASSERT_TRUE(curve.isFinished());
}

void test_weights_are_rounded_to_0_1_g_and_never_below_0(void)
{
    curve.add(200, 36.47f);
    curve.add(400, -0.5f);
    curve.finish(400);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 36.5f, curve.gramsAt(1));
    TEST_ASSERT_EQUAL_FLOAT(0, curve.gramsAt(2));
}

void test_reset_empties_it(void)
{
    feed(curve, 100, 6000, 100, 2);
    curve.finish(6000);
    curve.reset();
    TEST_ASSERT_EQUAL(0, curve.size());
    TEST_ASSERT_EQUAL(0, curve.durationMs());
    TEST_ASSERT_FALSE(curve.isFinished());
    TEST_ASSERT_EQUAL_FLOAT(0, curve.yieldGrams());
}

void test_finish_without_readings_is_flat(void)
{
    curve.finish(6000);
    TEST_ASSERT_EQUAL(31, curve.size());
    for (size_t i = 0; i < curve.size(); i++)
    {
        TEST_ASSERT_EQUAL_FLOAT(0, curve.gramsAt(i));
    }
    TEST_ASSERT_FALSE(curve.isFinished());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_points_come_every_200_ms);
    RUN_TEST(test_a_point_takes_the_newest_reading_at_or_before_it);
    RUN_TEST(test_flat_until_the_first_reading);
    RUN_TEST(test_points_after_the_end_are_dropped);
    RUN_TEST(test_the_end_fills_up_to_it);
    RUN_TEST(test_the_curve_stops_at_90_s);
    RUN_TEST(test_the_last_point_is_the_newest_reading_at_or_before_the_end);
    RUN_TEST(test_the_last_point_is_the_weight_at_a_late_end);
    RUN_TEST(test_short_or_light_shots_are_not_finished);
    RUN_TEST(test_weights_are_rounded_to_0_1_g_and_never_below_0);
    RUN_TEST(test_reset_empties_it);
    RUN_TEST(test_finish_without_readings_is_flat);
    return UNITY_END();
}
