#include <unity.h>

#include <string>

#include "recorder.h"

static Recorder *recorder;
static std::string csv;

static void collect(const char *data, size_t length) { csv.append(data, length); }

void setUp(void)
{
    recorder = new Recorder();
    csv.clear();
}

void tearDown(void) { delete recorder; }

void test_counts_samples_and_clicks(void)
{
    recorder->start(1000);
    recorder->addSample(1000, 84000);
    recorder->addSample(1100, 84010);
    recorder->addClick(1150);

    TEST_ASSERT_EQUAL(2, recorder->sampleCount());
    TEST_ASSERT_EQUAL(1, recorder->clickCount());
    TEST_ASSERT_EQUAL(150, recorder->durationMs());
    TEST_ASSERT_EQUAL(299850, recorder->remainingMs());
}

void test_csv(void)
{
    recorder->start(5000);
    recorder->addSample(5000, 84012);
    recorder->addSample(5101, 84020);
    recorder->addClick(5234);

    size_t written = recorder->writeCsv("v1.9.0", 0.5f, collect);

    TEST_ASSERT_EQUAL_STRING("# coffee-scale recording\n"
                             "# firmware: v1.9.0\n"
                             "# grams_per_count: 0.5\n"
                             "ms,event,value\n"
                             "0,sample,84012\n"
                             "101,sample,84020\n"
                             "234,click,\n",
                             csv.c_str());
    TEST_ASSERT_EQUAL(csv.size(), written);
}

void test_csv_length_matches_written_bytes(void)
{
    recorder->start(0);
    for (unsigned long ms = 0; ms < 60000; ms += 100)
    {
        recorder->addSample(ms, 16777215 - ms);
    }
    recorder->addClick(12345);

    size_t counted = 0;
    size_t countedTotal = recorder->writeCsv("v1.9.0", 0.002381f, [&](const char *, size_t length) { counted += length; });
    size_t written = recorder->writeCsv("v1.9.0", 0.002381f, collect);

    TEST_ASSERT_EQUAL(counted, countedTotal);
    TEST_ASSERT_EQUAL(countedTotal, written);
    TEST_ASSERT_EQUAL(written, csv.size());
}

static int countRows(const std::string &text)
{
    int rows = 0;
    size_t header = text.find("ms,event,value\n");
    for (size_t i = header + 15; i < text.size(); i++)
    {
        if (text[i] == '\n')
        {
            rows++;
        }
    }
    return rows;
}

void test_full_drops_further_events(void)
{
    Recorder small(3);
    small.start(0);
    small.addSample(0, 1);
    small.addSample(100, 2);
    small.addSample(200, 3);
    TEST_ASSERT_TRUE(small.isFull());

    small.addSample(300, 4);
    small.addClick(350);

    TEST_ASSERT_EQUAL(3, small.sampleCount());
    TEST_ASSERT_EQUAL(0, small.clickCount());
    small.writeCsv("v1.9.0", 1, collect);
    TEST_ASSERT_EQUAL(3, countRows(csv));
    TEST_ASSERT_EQUAL(200, small.durationMs());
}

void test_full_after_five_minutes(void)
{
    recorder->start(1000);
    recorder->addSample(1000, 1);
    recorder->addSample(1000 + 299999, 2);
    TEST_ASSERT_FALSE(recorder->isFull());

    recorder->addSample(1000 + 300000, 3);
    TEST_ASSERT_TRUE(recorder->isFull());
    TEST_ASSERT_EQUAL(0, recorder->remainingMs());

    recorder->addSample(1000 + 300100, 4);
    TEST_ASSERT_EQUAL(3, recorder->sampleCount());
}

void test_start_clears(void)
{
    Recorder small(2);
    small.start(0);
    small.addSample(0, 1);
    small.addClick(50);
    TEST_ASSERT_TRUE(small.isFull());

    small.start(10000);

    TEST_ASSERT_EQUAL(0, small.sampleCount());
    TEST_ASSERT_EQUAL(0, small.clickCount());
    TEST_ASSERT_EQUAL(0, small.durationMs());
    TEST_ASSERT_FALSE(small.isFull());
}

void test_allocation(void)
{
    TEST_ASSERT_TRUE(recorder->isAllocated());
    Recorder none(0);
    TEST_ASSERT_FALSE(none.isAllocated());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_counts_samples_and_clicks);
    RUN_TEST(test_csv);
    RUN_TEST(test_csv_length_matches_written_bytes);
    RUN_TEST(test_full_drops_further_events);
    RUN_TEST(test_full_after_five_minutes);
    RUN_TEST(test_start_clears);
    RUN_TEST(test_allocation);
    return UNITY_END();
}
