#include <string.h>
#include <unity.h>

#include <string>
#include <vector>

#include "network/dns_answer.h"

static const uint8_t IP[4] = {192, 168, 4, 1};
static const uint16_t TYPE_A = 1;
static const uint16_t TYPE_AAAA = 28;

void setUp(void) {}

void tearDown(void) {}

/** A query for name, as a phone sends it: id 0x1234, recursion desired, one question. */
static std::vector<uint8_t> query(const char *name, uint16_t type)
{
    std::vector<uint8_t> q = {0x12, 0x34, 0x01, 0x00, 0, 1, 0, 0, 0, 0, 0, 0};
    const char *label = name;
    while (*label)
    {
        const char *dot = strchr(label, '.');
        size_t length = dot ? (size_t)(dot - label) : strlen(label);
        q.push_back((uint8_t)length);
        q.insert(q.end(), label, label + length);
        label += length + (dot ? 1 : 0);
    }
    q.push_back(0);
    q.push_back(type >> 8);
    q.push_back(type & 0xFF);
    q.push_back(0);
    q.push_back(1); // class IN
    return q;
}

static size_t reply(const std::vector<uint8_t> &q, uint8_t *out, size_t outSize = 512)
{
    return DnsAnswer::reply(q.data(), q.size(), IP, out, outSize);
}

void test_a_query_gets_the_scales_address(void)
{
    std::vector<uint8_t> q = query("captive.apple.com", TYPE_A);
    uint8_t out[512];
    size_t length = reply(q, out);

    TEST_ASSERT_EQUAL(q.size() + 16, length);
    TEST_ASSERT_EQUAL_HEX8(0x12, out[0]); // the query's id
    TEST_ASSERT_EQUAL_HEX8(0x34, out[1]);
    TEST_ASSERT_EQUAL_HEX8(0x85, out[2]); // response, authoritative, recursion desired
    TEST_ASSERT_EQUAL_HEX8(0x80, out[3]); // recursion available, no error
    TEST_ASSERT_EQUAL(1, out[5]);         // one question
    TEST_ASSERT_EQUAL(1, out[7]);         // one answer
    TEST_ASSERT_EQUAL_MEMORY(q.data() + 12, out + 12, q.size() - 12);
    const uint8_t answer[16] = {0xC0, 0x0C, 0, 1, 0, 1, 0, 0, 0, 60, 0, 4, 192, 168, 4, 1};
    TEST_ASSERT_EQUAL_MEMORY(answer, out + q.size(), 16);
}

void test_other_types_get_no_answer(void)
{
    std::vector<uint8_t> q = query("connectivitycheck.gstatic.com", TYPE_AAAA);
    uint8_t out[512];
    TEST_ASSERT_EQUAL(q.size(), reply(q, out));
    TEST_ASSERT_EQUAL_HEX8(0x80, out[3]); // no error, just nothing
    TEST_ASSERT_EQUAL(0, out[7]);
}

// e.g. EDNS: the reply leaves them out
void test_additional_records_are_left_out(void)
{
    std::vector<uint8_t> q = query("example.com", TYPE_A);
    const size_t question = q.size();
    q[11] = 1;
    const uint8_t opt[] = {0, 0, 41, 0x10, 0, 0, 0, 0, 0, 0, 0};
    q.insert(q.end(), opt, opt + sizeof(opt));
    uint8_t out[512];

    TEST_ASSERT_EQUAL(question + 16, reply(q, out));
    TEST_ASSERT_EQUAL(0, out[11]);
}

void test_responses_get_no_reply(void)
{
    std::vector<uint8_t> q = query("example.com", TYPE_A);
    q[2] |= 0x80;
    uint8_t out[512];
    TEST_ASSERT_EQUAL(0, reply(q, out));
}

void test_other_opcodes_get_no_reply(void)
{
    std::vector<uint8_t> q = query("example.com", TYPE_A);
    q[2] |= 0x10; // opcode 2, status
    uint8_t out[512];
    TEST_ASSERT_EQUAL(0, reply(q, out));
}

void test_two_questions_get_no_reply(void)
{
    std::vector<uint8_t> q = query("example.com", TYPE_A);
    q[5] = 2;
    uint8_t out[512];
    TEST_ASSERT_EQUAL(0, reply(q, out));
}

void test_truncated_queries_get_no_reply(void)
{
    std::vector<uint8_t> q = query("example.com", TYPE_A);
    uint8_t out[512];
    for (size_t length = 0; length < q.size(); length++)
    {
        TEST_ASSERT_EQUAL_MESSAGE(0, DnsAnswer::reply(q.data(), length, IP, out, sizeof(out)),
                                  std::to_string(length).c_str());
    }
}

void test_a_label_past_the_end_gets_no_reply(void)
{
    std::vector<uint8_t> q = {0x12, 0x34, 0x01, 0x00, 0, 1, 0, 0, 0, 0, 0, 0, 63, 'a', 'b'};
    uint8_t out[512];
    TEST_ASSERT_EQUAL(0, reply(q, out));
}

// a question never points elsewhere, and following pointers could loop
void test_compressed_names_get_no_reply(void)
{
    std::vector<uint8_t> q = {0x12, 0x34, 0x01, 0x00, 0, 1, 0, 0, 0, 0, 0, 0, 0xC0, 0x0C, 0, 1, 0, 1};
    uint8_t out[512];
    TEST_ASSERT_EQUAL(0, reply(q, out));
}

void test_the_reply_has_to_fit(void)
{
    std::vector<uint8_t> q = query("example.com", TYPE_A);
    uint8_t out[512];
    TEST_ASSERT_EQUAL(0, reply(q, out, q.size() + 15));
    TEST_ASSERT_EQUAL(q.size() + 16, reply(q, out, q.size() + 16));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_a_query_gets_the_scales_address);
    RUN_TEST(test_other_types_get_no_answer);
    RUN_TEST(test_additional_records_are_left_out);
    RUN_TEST(test_responses_get_no_reply);
    RUN_TEST(test_other_opcodes_get_no_reply);
    RUN_TEST(test_two_questions_get_no_reply);
    RUN_TEST(test_truncated_queries_get_no_reply);
    RUN_TEST(test_a_label_past_the_end_gets_no_reply);
    RUN_TEST(test_compressed_names_get_no_reply);
    RUN_TEST(test_the_reply_has_to_fit);
    return UNITY_END();
}
