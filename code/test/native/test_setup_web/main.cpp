#include <string.h>
#include <unity.h>

#include <string>

#include "network/setup_web.h"

using SetupWeb::Network;

void setUp(void) {}

void tearDown(void) {}

static bool field(const char *body, const char *key, std::string &value, size_t maxLength = 64)
{
    return SetupWeb::formField(body, strlen(body), key, value, maxLength);
}

void test_fields_are_found_by_key(void)
{
    std::string ssid, password;
    TEST_ASSERT_TRUE(field("ssid=Home&password=secret", "ssid", ssid));
    TEST_ASSERT_TRUE(field("ssid=Home&password=secret", "password", password));
    TEST_ASSERT_EQUAL_STRING("Home", ssid.c_str());
    TEST_ASSERT_EQUAL_STRING("secret", password.c_str());
}

// what URLSearchParams sends for the password "p&s=w+o%rd ä"
void test_special_characters_arrive_unchanged(void)
{
    std::string password;
    TEST_ASSERT_TRUE(field("ssid=x&password=p%26s%3Dw%2Bo%25rd+%C3%A4", "password", password));
    TEST_ASSERT_EQUAL_STRING("p&s=w+o%rd \xC3\xA4", password.c_str());
}

void test_a_missing_field_is_not_found(void)
{
    std::string value;
    TEST_ASSERT_FALSE(field("ssid=Home", "password", value));
    TEST_ASSERT_FALSE(field("ssid=Home&password", "password", value));
}

void test_an_empty_field_is_found(void)
{
    std::string value = "old";
    TEST_ASSERT_TRUE(field("ssid=Open&password=", "password", value));
    TEST_ASSERT_EQUAL_STRING("", value.c_str());
}

void test_a_key_is_not_matched_by_its_prefix(void)
{
    std::string value;
    TEST_ASSERT_TRUE(field("ssidx=a&ssid=b", "ssid", value));
    TEST_ASSERT_EQUAL_STRING("b", value.c_str());
}

void test_broken_percent_encoding_is_rejected(void)
{
    std::string value;
    TEST_ASSERT_FALSE(field("ssid=a%4", "ssid", value));
    TEST_ASSERT_FALSE(field("ssid=a%zz", "ssid", value));
}

// a Wi-Fi name has at most 32 bytes
void test_fields_longer_than_allowed_are_rejected(void)
{
    std::string value;
    TEST_ASSERT_TRUE(field("ssid=12345678901234567890123456789012", "ssid", value, 32));
    TEST_ASSERT_FALSE(field("ssid=123456789012345678901234567890123", "ssid", value, 32));
}

// the HTTP server's buffer isn't null-terminated
void test_only_length_bytes_are_read(void)
{
    std::string value;
    TEST_ASSERT_TRUE(SetupWeb::formField("ssid=Home&more", 9, "ssid", value, 32));
    TEST_ASSERT_EQUAL_STRING("Home", value.c_str());
}

void test_networks_are_sorted_strongest_first(void)
{
    std::string json = SetupWeb::networksJson({{"Far", -80, true}, {"Near", -40, false}});
    TEST_ASSERT_EQUAL_STRING("[{\"ssid\":\"Near\",\"rssi\":-40,\"secure\":false},"
                             "{\"ssid\":\"Far\",\"rssi\":-80,\"secure\":true}]",
                             json.c_str());
}

void test_hidden_networks_are_left_out(void)
{
    std::string json = SetupWeb::networksJson({{"", -30, true}, {"Home", -50, true}});
    TEST_ASSERT_EQUAL_STRING("[{\"ssid\":\"Home\",\"rssi\":-50,\"secure\":true}]", json.c_str());
}

// e.g. a mesh with several access points
void test_a_name_is_listed_once_with_its_strongest_signal(void)
{
    std::string json = SetupWeb::networksJson({{"Mesh", -70, true}, {"Other", -60, true}, {"Mesh", -50, true}});
    TEST_ASSERT_EQUAL_STRING("[{\"ssid\":\"Mesh\",\"rssi\":-50,\"secure\":true},"
                             "{\"ssid\":\"Other\",\"rssi\":-60,\"secure\":true}]",
                             json.c_str());
}

void test_names_are_escaped(void)
{
    TEST_ASSERT_EQUAL_STRING("\"a\\\"b\\\\c\\u0001d\"", SetupWeb::jsonString("a\"b\\c\x01" "d").c_str());
    // UTF-8 stays as it is
    TEST_ASSERT_EQUAL_STRING("\"Caf\xC3\xA9\"", SetupWeb::jsonString("Caf\xC3\xA9").c_str());
}

void test_no_networks_is_an_empty_list(void)
{
    TEST_ASSERT_EQUAL_STRING("[]", SetupWeb::networksJson({}).c_str());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_fields_are_found_by_key);
    RUN_TEST(test_special_characters_arrive_unchanged);
    RUN_TEST(test_a_missing_field_is_not_found);
    RUN_TEST(test_an_empty_field_is_found);
    RUN_TEST(test_a_key_is_not_matched_by_its_prefix);
    RUN_TEST(test_broken_percent_encoding_is_rejected);
    RUN_TEST(test_fields_longer_than_allowed_are_rejected);
    RUN_TEST(test_only_length_bytes_are_read);
    RUN_TEST(test_networks_are_sorted_strongest_first);
    RUN_TEST(test_hidden_networks_are_left_out);
    RUN_TEST(test_a_name_is_listed_once_with_its_strongest_signal);
    RUN_TEST(test_names_are_escaped);
    RUN_TEST(test_no_networks_is_an_empty_list);
    return UNITY_END();
}
