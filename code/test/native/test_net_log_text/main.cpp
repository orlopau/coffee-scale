#include <string.h>
#include <unity.h>

#include <string>

#include "net_log_text.h"

void setUp(void) {}

void tearDown(void) {}

static std::string strip(std::string s)
{
    size_t n = NetLogText::stripColors(&s[0], s.size());
    return s.substr(0, n);
}

void test_strips_esp_log_colors()
{
    TEST_ASSERT_EQUAL_STRING("I (123) main: hi\n", strip("\033[0;32mI (123) main: hi\033[0m\n").c_str());
}

void test_plain_text_unchanged()
{
    TEST_ASSERT_EQUAL_STRING("W (1) x: [ok]\n", strip("W (1) x: [ok]\n").c_str());
}

void test_cut_off_sequence_removed()
{
    TEST_ASSERT_EQUAL_STRING("E (9) x: y", strip("E (9) x: y\033[0;3").c_str());
}

void test_lone_escape_at_end_removed()
{
    TEST_ASSERT_EQUAL_STRING("a", strip("a\033").c_str());
}

void test_dropped_line()
{
    char out[64];
    size_t n = NetLogText::droppedLine(17, out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING("net_log: 17 lines dropped\n", out);
    TEST_ASSERT_EQUAL(strlen(out), n);
}

void test_dropped_line_truncates()
{
    char out[8];
    size_t n = NetLogText::droppedLine(17, out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING("net_log", out);
    TEST_ASSERT_EQUAL(7, n);
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_strips_esp_log_colors);
    RUN_TEST(test_plain_text_unchanged);
    RUN_TEST(test_cut_off_sequence_removed);
    RUN_TEST(test_lone_escape_at_end_removed);
    RUN_TEST(test_dropped_line);
    RUN_TEST(test_dropped_line_truncates);
    return UNITY_END();
}
