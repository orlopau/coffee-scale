#include <unity.h>

#include "auto_tare.h"

AutoTare *autoTare;

void setUp(void)
{
    // tolerance 0.1 and std dev 1g, 5 buffer size
    autoTare = new AutoTare(0.1, 1, 5);
    // add a weight of 36g
    autoTare->weights.push_back(36.0);
}

void tearDown(void) { delete autoTare; }

void test_auto_tare(void)
{
    float fluctuatingWeights[] = {100.0, 101.0, 99.5, 100.5, 100.0, 99.8, 100.2};

    for (float weight : fluctuatingWeights)
    {
        autoTare->update(weight);
    }

    // now weighht shall be stable
    // but shouldnt tare yet
    TEST_ASSERT_FALSE(autoTare->shouldTare());

    // now the weight increases quite fast, with 5 stable values for the buffer
    float newWeight[] = {101.0, 130.0, 135.0, 136.0, 135.5, 135.8, 136.1, 136.1, 136.1, 136.1, 136.1};

    for (float weight : newWeight)
    {
        autoTare->update(weight);
    }

    // the weight is stable and should trigger tare
    TEST_ASSERT_TRUE(autoTare->shouldTare());
    // after taring, its false again
    TEST_ASSERT_FALSE(autoTare->shouldTare());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_auto_tare);
    UNITY_END();
}