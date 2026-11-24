#include <cmath>
#include <string>
#include <unity.h>

#include "grinder/xenia.h"

void setUp(void) {}
void tearDown(void) {}

static bool has(const std::string &body, const char *part)
{
    return body.find(part) != std::string::npos;
}

void test_ready_reply(void)
{
    Xenia::Machine m;
    TEST_ASSERT_EQUAL_STRING(
        "{\"MA_STATUS\":1,\"MA_EXTRACTION_STATUS\":0,\"REAL_EXTRACTION_TIME_MS\":0,"
        "\"PU_SENS_FLOW_METER_VOLUME\":0.0,\"BG_SET_TEMP\":93.0,\"BG_SENS_TEMP\":93.0,"
        "\"TANK_LEVEL\":1,\"script\":\"\",\"TIMESTAMP\":\"\"}",
        m.mako(0).c_str());
}

void test_a_shot(void)
{
    Xenia::Machine m;
    m.start(1000);
    std::string running = m.mako(3000);
    TEST_ASSERT_TRUE(has(running, "\"MA_STATUS\":3"));
    TEST_ASSERT_TRUE(has(running, "\"MA_EXTRACTION_STATUS\":1"));
    TEST_ASSERT_TRUE(has(running, "\"REAL_EXTRACTION_TIME_MS\":2000"));

    m.finish(30000, 27400, 40.2f);
    m.update(30000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Finishing, m.phase());
    TEST_ASSERT_TRUE(has(m.mako(31000),
                         "\"MA_STATUS\":4,\"MA_EXTRACTION_STATUS\":1,\"REAL_EXTRACTION_TIME_MS\":27400,"
                         "\"PU_SENS_FLOW_METER_VOLUME\":40.2"));

    m.update(36000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Ready, m.phase());
    std::string ready = m.mako(37000);
    TEST_ASSERT_TRUE(has(ready, "\"MA_STATUS\":1"));
    TEST_ASSERT_TRUE(has(ready, "27400"));
    TEST_ASSERT_TRUE(has(ready, "40.2"));
}

void test_abort(void)
{
    Xenia::Machine m;
    m.start(0);
    m.mako(1000);
    m.abort(5000);
    m.update(5000);
    std::string body = m.mako(6000);
    TEST_ASSERT_TRUE(has(body, "\"MA_STATUS\":4,\"MA_EXTRACTION_STATUS\":2"));
    TEST_ASSERT_TRUE(has(body, "\"PU_SENS_FLOW_METER_VOLUME\":0.0"));
    TEST_ASSERT_TRUE(m.aborted());
}

void test_abort_waits_until_the_grinder_saw_the_shot(void)
{
    Xenia::Machine m;
    m.start(0);
    m.abort(10);
    m.update(10);
    TEST_ASSERT_EQUAL(Xenia::Phase::Running, m.phase());
    TEST_ASSERT_TRUE(has(m.mako(500), "\"MA_STATUS\":3"));
    m.update(600);
    TEST_ASSERT_EQUAL(Xenia::Phase::Finishing, m.phase());
    TEST_ASSERT_TRUE(m.aborted());
}

void test_finishing_waits_for_a_poll(void)
{
    Xenia::Machine m;
    m.start(0);
    m.mako(100);
    m.finish(10000, 9000, 36.0f);
    m.update(10000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Finishing, m.phase());
    m.update(10000 + Xenia::FINISHING_HOLD_MS);
    TEST_ASSERT_EQUAL(Xenia::Phase::Finishing, m.phase());
    m.update(10000 + 59000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Finishing, m.phase());
    m.mako(10000 + 59000);
    m.update(10000 + 59001);
    TEST_ASSERT_EQUAL(Xenia::Phase::Ready, m.phase());
}

void test_finishing_ends_without_any_poll(void)
{
    Xenia::Machine m;
    m.start(0);
    m.mako(100);
    m.finish(10000, 9000, 36.0f);
    m.update(10000);
    m.update(10000 + Xenia::FINISHING_MAX_MS);
    TEST_ASSERT_EQUAL(Xenia::Phase::Ready, m.phase());
}

void test_finishing_counts_from_the_first_poll_of_the_shot(void)
{
    Xenia::Machine m;
    m.start(0);
    m.abort(10);
    m.mako(5000);
    m.update(5000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Finishing, m.phase());
    m.update(10999);
    TEST_ASSERT_EQUAL(Xenia::Phase::Finishing, m.phase());
    m.mako(10999);
    m.update(11000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Ready, m.phase());
}

void test_finishing_shows_after_polls_were_lost(void)
{
    Xenia::Machine m;
    m.start(0);
    m.finish(1000, 900, 36.0f);
    m.update(70000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Running, m.phase());
    m.mako(70000);
    m.update(70000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Finishing, m.phase());
    m.update(70001);
    TEST_ASSERT_EQUAL(Xenia::Phase::Finishing, m.phase());
    m.mako(70002);
    m.update(76000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Ready, m.phase());
}

void test_the_first_end_wins(void)
{
    Xenia::Machine m;
    m.start(0);
    m.mako(100);
    m.finish(1000, 900, 36.0f);
    m.abort(1001);
    m.finish(1002, 5, 1.0f);
    m.update(1002);
    TEST_ASSERT_FALSE(m.aborted());
    TEST_ASSERT_TRUE(has(m.mako(1003), "\"REAL_EXTRACTION_TIME_MS\":900,\"PU_SENS_FLOW_METER_VOLUME\":36.0"));
}

void test_a_weight_that_is_not_a_number_prints_as_zero(void)
{
    Xenia::Machine m;
    m.start(0);
    m.mako(100);
    m.finish(1000, 900, NAN);
    m.update(1000);
    TEST_ASSERT_TRUE(has(m.mako(1001), "\"PU_SENS_FLOW_METER_VOLUME\":0.0"));
}

void test_timeout(void)
{
    Xenia::Machine m;
    m.start(0);
    m.mako(100);
    m.update(179999);
    TEST_ASSERT_EQUAL(Xenia::Phase::Running, m.phase());
    m.update(180000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Finishing, m.phase());
    TEST_ASSERT_TRUE(m.aborted());
}

void test_no_timeout_once_the_shot_runs(void)
{
    Xenia::Machine m;
    m.start(0);
    m.mako(100);
    m.shotRuns();
    m.update(400000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Running, m.phase());
    m.finish(401000, 30000, 40.0f);
    m.update(401000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Finishing, m.phase());
    TEST_ASSERT_FALSE(m.aborted());
}

void test_the_timeout_counts_again_for_the_next_shot(void)
{
    Xenia::Machine m;
    m.start(0);
    m.mako(100);
    m.shotRuns();
    m.finish(30000, 28000, 40.0f);
    m.update(30000);
    m.mako(31000);
    m.update(36000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Ready, m.phase());
    m.start(40000);
    m.mako(41000);
    m.update(220000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Finishing, m.phase());
    TEST_ASSERT_TRUE(m.aborted());
}

void test_start_from_ready_or_finishing(void)
{
    Xenia::Machine m;
    m.start(1000);
    m.start(5000);
    TEST_ASSERT_TRUE(has(m.mako(6000), "\"REAL_EXTRACTION_TIME_MS\":5000"));
    m.finish(7000, 6000, 40.0f);
    m.update(7000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Finishing, m.phase());
    // a knob press while finishing ends it
    m.start(21000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Running, m.phase());
    std::string running = m.mako(22000);
    TEST_ASSERT_TRUE(has(running, "\"MA_STATUS\":3"));
    TEST_ASSERT_TRUE(has(running, "\"REAL_EXTRACTION_TIME_MS\":1000"));
}

// the E64 WS sent each knob press three times, 8.2-8.8 s apart, also while it saw Running (recorded 2026-11-24)
void test_repeats_of_the_knob_press_are_one_press(void)
{
    Xenia::Machine m;
    m.start(0);
    m.mako(800);
    // a shot over before the repeats, e.g. aborted
    m.abort(2000);
    m.update(2000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Finishing, m.phase());
    m.start(8700);
    TEST_ASSERT_EQUAL(Xenia::Phase::Finishing, m.phase());
    m.mako(9000);
    m.update(9000);
    TEST_ASSERT_EQUAL(Xenia::Phase::Ready, m.phase());
    m.start(16900);
    TEST_ASSERT_EQUAL(Xenia::Phase::Ready, m.phase());
    // the next knob press
    m.start(Xenia::START_REPEAT_MS);
    TEST_ASSERT_EQUAL(Xenia::Phase::Running, m.phase());
    TEST_ASSERT_TRUE(has(m.mako(Xenia::START_REPEAT_MS + 500), "\"REAL_EXTRACTION_TIME_MS\":500"));
}

void test_repeats_while_running_keep_the_time(void)
{
    Xenia::Machine m;
    m.start(0);
    m.mako(800);
    m.start(8700);
    m.start(16900);
    TEST_ASSERT_TRUE(has(m.mako(25000), "\"REAL_EXTRACTION_TIME_MS\":25000"));
}

void test_brew_weight_survives_start(void)
{
    Xenia::Machine m;
    TEST_ASSERT_TRUE(std::isnan(m.brewWeight()));
    m.setBrewWeight(38.0f);
    m.start(0);
    TEST_ASSERT_EQUAL_FLOAT(38.0f, m.brewWeight());
}

void test_identity(void)
{
    TEST_ASSERT_EQUAL_STRING(
        "{\"MA_TYPE\":1,\"MA_SUBTYPE\":0,\"MA_SN\":\"CS1A2B3C4D\",\"FW_VERSION_MAJOR\":4,"
        "\"FW_VERSION_MINOR\":226,\"ESP_FW_MAJOR\":3,\"ESP_FW_MINOR\":87,\"MA_MAX_AMPERE\":13,"
        "\"MA_FIX_WATER_SUPPLY\":0,\"BLE\":0,\"MA_DELAY\":-1}",
        Xenia::identityJson("CS1A2B3C4D").c_str());
}

void test_classify(void)
{
    TEST_ASSERT_EQUAL(Xenia::Request::Mako, Xenia::classify("GET", "/api/v2/mako"));
    TEST_ASSERT_EQUAL(Xenia::Request::Mako, Xenia::classify("get", "/api/v2/mako/"));
    TEST_ASSERT_EQUAL(Xenia::Request::Mako, Xenia::classify("GET", "/api/v2/mako?x=1"));
    TEST_ASSERT_EQUAL(Xenia::Request::Identity, Xenia::classify("GET", "/api/v2/machine"));
    TEST_ASSERT_EQUAL(Xenia::Request::BrewRatio, Xenia::classify("POST", "/api/v2/brewratio"));
    TEST_ASSERT_EQUAL(Xenia::Request::StartBrew, Xenia::classify("POST", "/api/v2/scripts/execute"));
    TEST_ASSERT_EQUAL(Xenia::Request::Other, Xenia::classify("POST", "/api/v2/mako"));
    TEST_ASSERT_EQUAL(Xenia::Request::Other, Xenia::classify("GET", "/"));
}

void test_brew_weight_parsing(void)
{
    TEST_ASSERT_EQUAL_FLOAT(40.0f, Xenia::brewWeightFrom("{\"SYNC_BEVERAGE_WEIGHT\":40}"));
    TEST_ASSERT_EQUAL_FLOAT(36.5f, Xenia::brewWeightFrom("{\"SYNC_FILTER\":2,\"SYNC_BEVERAGE_WEIGHT\": 36.5}"));
    TEST_ASSERT_EQUAL_FLOAT(38.0f, Xenia::brewWeightFrom("{\"SYNC_BEVERAGE_WEIGHT\":\"38\"}"));
    TEST_ASSERT_TRUE(std::isnan(Xenia::brewWeightFrom("{}")));
    TEST_ASSERT_TRUE(std::isnan(Xenia::brewWeightFrom("{\"SYNC_BEVERAGE_WEIGHT\":0}")));
    TEST_ASSERT_TRUE(std::isnan(Xenia::brewWeightFrom("{\"SYNC_BEVERAGE_WEIGHT\":-3}")));
    TEST_ASSERT_TRUE(std::isnan(Xenia::brewWeightFrom("{\"SYNC_BEVERAGE_WEIGHT\":\"x\"}")));
    TEST_ASSERT_TRUE(std::isnan(Xenia::brewWeightFrom("not json")));
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_ready_reply);
    RUN_TEST(test_a_shot);
    RUN_TEST(test_abort);
    RUN_TEST(test_abort_waits_until_the_grinder_saw_the_shot);
    RUN_TEST(test_finishing_waits_for_a_poll);
    RUN_TEST(test_finishing_ends_without_any_poll);
    RUN_TEST(test_finishing_counts_from_the_first_poll_of_the_shot);
    RUN_TEST(test_finishing_shows_after_polls_were_lost);
    RUN_TEST(test_the_first_end_wins);
    RUN_TEST(test_a_weight_that_is_not_a_number_prints_as_zero);
    RUN_TEST(test_timeout);
    RUN_TEST(test_no_timeout_once_the_shot_runs);
    RUN_TEST(test_the_timeout_counts_again_for_the_next_shot);
    RUN_TEST(test_start_from_ready_or_finishing);
    RUN_TEST(test_repeats_of_the_knob_press_are_one_press);
    RUN_TEST(test_repeats_while_running_keep_the_time);
    RUN_TEST(test_brew_weight_survives_start);
    RUN_TEST(test_identity);
    RUN_TEST(test_classify);
    RUN_TEST(test_brew_weight_parsing);
    return UNITY_END();
}
