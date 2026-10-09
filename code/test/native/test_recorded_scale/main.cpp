#include <unity.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "native/recorded_scale.h"
#include "millis.h"
#include "recorder.h"

namespace fs = std::filesystem;

static const fs::path FIXTURES = fs::path(__FILE__).parent_path() / "fixtures";

static std::string fixture(const char *name) { return (FIXTURES / name).string(); }

void setUp(void) { set_now(1000000); }

void tearDown(void) {}

void test_header(void)
{
    RecordedScale scale(fixture("basic.csv"));

    TEST_ASSERT_EQUAL_STRING("v1.9.0", scale.firmware().c_str());
    TEST_ASSERT_EQUAL_FLOAT(0.0025f, scale.gramsPerCount());
    TEST_ASSERT_EQUAL(1, scale.clicks().size());
    TEST_ASSERT_EQUAL(150, scale.clicks()[0]);
}

void test_samples_at_recorded_times(void)
{
    RecordedScale scale(fixture("basic.csv"));
    scale.update();

    TEST_ASSERT_TRUE(scale.isReady());
    TEST_ASSERT_EQUAL(1000, scale.read());
    TEST_ASSERT_FALSE(scale.isReady());

    sleep_for(99);
    TEST_ASSERT_FALSE(scale.isReady());
    sleep_for(1);
    TEST_ASSERT_TRUE(scale.isReady());
    TEST_ASSERT_EQUAL(1010, scale.read());
    TEST_ASSERT_EQUAL(100, scale.elapsed());
}

void test_starts_without_update(void)
{
    RecordedScale scale(fixture("basic.csv"));
    sleep_for(5000); // loading takes time in a test too, the replay starts at the first call

    TEST_ASSERT_TRUE(scale.isReady());
    TEST_ASSERT_EQUAL(1000, scale.read());
    TEST_ASSERT_FALSE(scale.isReady());
}

void test_time_jump_returns_latest(void)
{
    RecordedScale scale(fixture("basic.csv"));
    scale.update();
    sleep_for(250);

    TEST_ASSERT_EQUAL(1020, scale.read());
    TEST_ASSERT_FALSE(scale.isReady());
}

void test_finished(void)
{
    RecordedScale scale(fixture("basic.csv"));
    scale.update();
    sleep_for(300);
    TEST_ASSERT_FALSE(scale.isFinished());

    TEST_ASSERT_EQUAL(1030, scale.read());
    TEST_ASSERT_TRUE(scale.isFinished());
    sleep_for(1000);
    TEST_ASSERT_FALSE(scale.isReady());
    TEST_ASSERT_EQUAL(1030, scale.read());
}

void test_unknown_event_skipped(void)
{
    RecordedScale scale(fixture("basic.csv"));
    std::vector<long> values;
    for (int ms = 0; ms <= 1000; ms++)
    {
        if (scale.isReady())
        {
            values.push_back(scale.read());
        }
        sleep_for(1);
    }

    TEST_ASSERT_EQUAL(4, values.size());
    TEST_ASSERT_EQUAL(1030, values[3]);
}

static void assertError(const char *name, int line)
{
    std::string expected = std::string(name) + ":" + std::to_string(line) + ":";
    try
    {
        RecordedScale scale(fixture(name));
        TEST_FAIL_MESSAGE(("no error for " + std::string(name)).c_str());
    }
    catch (const std::runtime_error &e)
    {
        TEST_ASSERT_NOT_NULL_MESSAGE(strstr(e.what(), expected.c_str()), e.what());
    }
}

void test_errors(void)
{
    assertError("no_header.csv", 1);
    assertError("no_scale.csv", 3);
    assertError("no_columns.csv", 4);
    assertError("bad_ms.csv", 6);
    assertError("backwards.csv", 7);
    assertError("sample_without_value.csv", 6);
}

void test_missing_file(void)
{
    try
    {
        RecordedScale scale(fixture("does_not_exist.csv"));
        TEST_FAIL_MESSAGE("no error for a missing file");
    }
    catch (const std::runtime_error &e)
    {
        TEST_ASSERT_NOT_NULL(strstr(e.what(), "does_not_exist.csv"));
    }
}

void test_roundtrip_with_recorder(void)
{
    Recorder recorder;
    recorder.start(500);
    recorder.addSample(500, 0);
    recorder.addSample(600, 16777215);
    recorder.addClick(650);
    recorder.addSample(700, 8388608);

    fs::path path = fs::temp_directory_path() / "coffee-scale-roundtrip.csv";
    {
        std::ofstream file(path, std::ios::binary);
        recorder.writeCsv("v1.9.0", 0.002381f, [&](const char *data, size_t length) { file.write(data, length); });
    }

    RecordedScale scale(path.string());
    fs::remove(path);

    TEST_ASSERT_EQUAL_STRING("v1.9.0", scale.firmware().c_str());
    TEST_ASSERT_EQUAL_FLOAT(0.002381f, scale.gramsPerCount());
    TEST_ASSERT_EQUAL(1, scale.clicks().size());
    TEST_ASSERT_EQUAL(150, scale.clicks()[0]);

    scale.update();
    TEST_ASSERT_EQUAL(0, scale.read());
    sleep_for(100);
    TEST_ASSERT_EQUAL(16777215, scale.read());
    sleep_for(100);
    TEST_ASSERT_EQUAL(8388608, scale.read());
    TEST_ASSERT_TRUE(scale.isFinished());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_header);
    RUN_TEST(test_samples_at_recorded_times);
    RUN_TEST(test_starts_without_update);
    RUN_TEST(test_time_jump_returns_latest);
    RUN_TEST(test_finished);
    RUN_TEST(test_unknown_event_skipped);
    RUN_TEST(test_errors);
    RUN_TEST(test_missing_file);
    RUN_TEST(test_roundtrip_with_recorder);
    return UNITY_END();
}
