#include <regression.h>
#include <unity.h>

Regression::Approximator approximation(20);

template <size_t N>
void assertSlope(const Regression::Point (&points)[N], const float expectedSlope, const float expectedYIntercept)
{
    for (auto p : points)
    {
        approximation.addPoint(p);
    }

    Regression::Result result = approximation.getLeastSquares();
    TEST_ASSERT_EQUAL_FLOAT(expectedSlope, result.m);
    TEST_ASSERT_EQUAL_FLOAT(expectedYIntercept, result.yIntercept);
}

void setUp(void)
{
    approximation.reset();
}

void test_full(void)
{
    const Regression::Point pointsFull[] = {
        {0, 0},
        {1, 2},
        {3, 6},
        {8, 16},
        {10, 20},
        {20, 40},
        {22, 44},
        {1, 2},
        {2, 4},
        {6, 12},
    };

    assertSlope(pointsFull, 2.0, 0);
}

void test_not_full(void)
{
    const Regression::Point pointsNotFull[] = {
        {0, 0},
        {6, 12},
    };

    assertSlope(pointsNotFull, 2.0, 0);
}

void test_overflow_buffer(void)
{
    const Regression::Point pointsOverflow[] = {
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {10, 20},
        {20, 40},
        {22, 44},
    };

    assertSlope(pointsOverflow, 2.0, 0);
}

void test_floating_point(void)
{
    const Regression::Point pointsFP[] = {
        {2, 3.5},
        {4, 4.5},
    };

    assertSlope(pointsFP, 0.5, 2.5);
}

void test_size_variable(void)
{
    Regression::Approximator dualPointApproximation(2);
    const Regression::Point points[] = {
        {0, 0},
        {1, 1},
        {0, 50},
        {1, 100},
    };

    for (auto p : points)
    {
        dualPointApproximation.addPoint(p);
    }

    auto result = dualPointApproximation.getLeastSquares();
    TEST_ASSERT_EQUAL_FLOAT(50, result.m);
    TEST_ASSERT_EQUAL_FLOAT(50, result.yIntercept);
}

void test_x_at_y(void)
{
    const Regression::Point points[] = {
        {0, 1},
        {1, 3},
    };

    assertSlope(points, 2, 1);

    long x = approximation.getXAtY(7);
    TEST_ASSERT_EQUAL(3, x);
}

void test_empty(void)
{
    auto result = approximation.getLeastSquares();
    TEST_ASSERT_FLOAT_IS_NAN(result.m);
    TEST_ASSERT_FLOAT_IS_NAN(result.yIntercept);    
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_full);
    RUN_TEST(test_not_full);
    RUN_TEST(test_overflow_buffer);
    RUN_TEST(test_size_variable);
    RUN_TEST(test_x_at_y);
    RUN_TEST(test_empty);
    return UNITY_END();
}