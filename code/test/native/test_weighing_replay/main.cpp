#include <unity.h>

#include <filesystem>

#include "millis.h"
#include "native/recorded_scale.h"
#include "weighing/weight_sensor.h"

namespace fs = std::filesystem;

static const fs::path FIXTURES = fs::path(__FILE__).parent_path() / "fixtures";

/** A recording's samples, timestamped when they become ready, like the device's load cell task. */
class RecordedSamples : public SampleSource
{
public:
    explicit RecordedSamples(RecordedScale &scale) : scale(scale) {}

    bool next(Sample &sample) override
    {
        if (!scale.isReady())
        {
            return false;
        }
        sample = {static_cast<uint32_t>(now()), static_cast<int32_t>(scale.read())};
        return true;
    }

private:
    RecordedScale &scale;
};

void setUp(void) { set_now(1000000); }

void tearDown(void) {}

void test_pour_of_2_grams_per_second(void)
{
    RecordedScale scale((FIXTURES / "pour_2gs.csv").string());
    RecordedSamples samples(scale);
    DefaultWeightSensor weightSensor(samples);
    weightSensor.setScale(scale.gramsPerCount());

    // the empty cup for 1 s, then tare
    scale.update();
    for (int i = 0; i < 10; i++)
    {
        weightSensor.update();
        sleep_for(100);
    }
    weightSensor.tare();

    // 4 s of pouring
    for (int i = 0; i <= 40; i++)
    {
        weightSensor.update();
        sleep_for(100);
    }

    const Reading &reading = weightSensor.newReadings().back();
    TEST_ASSERT_FLOAT_WITHIN(0.05, 2, reading.flow);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 8, reading.net);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 108, reading.grams);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_pour_of_2_grams_per_second);
    return UNITY_END();
}
