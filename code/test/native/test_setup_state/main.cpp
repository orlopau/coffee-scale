#include <unity.h>

#include <string>

#include "network/setup_state.h"

void setUp(void) {}

void tearDown(void) {}

void test_starts_idle(void)
{
    SetupState state;
    std::string ssid, password;
    TEST_ASSERT_EQUAL(WifiStatus::Idle, state.status());
    TEST_ASSERT_FALSE(state.takeRequest(ssid, password));
}

// the page polls /status right after sending the network
void test_a_request_is_connecting_at_once(void)
{
    SetupState state;
    TEST_ASSERT_TRUE(state.request("Home", "secret"));
    TEST_ASSERT_EQUAL(WifiStatus::Connecting, state.status());
}

void test_the_loop_takes_a_request_once(void)
{
    SetupState state;
    state.request("Home", "secret");
    std::string ssid, password;
    TEST_ASSERT_TRUE(state.takeRequest(ssid, password));
    TEST_ASSERT_EQUAL_STRING("Home", ssid.c_str());
    TEST_ASSERT_EQUAL_STRING("secret", password.c_str());
    TEST_ASSERT_FALSE(state.takeRequest(ssid, password));
}

void test_no_new_request_while_connecting(void)
{
    SetupState state;
    state.request("Home", "secret");
    TEST_ASSERT_FALSE(state.request("Other", "x"));

    std::string ssid, password;
    state.takeRequest(ssid, password);
    TEST_ASSERT_FALSE(state.request("Other", "x"));
}

void test_after_a_failure_the_page_can_try_again(void)
{
    SetupState state;
    std::string ssid, password;
    state.request("Home", "wrong");
    state.takeRequest(ssid, password);
    state.finish(WifiStatus::WrongPassword);
    TEST_ASSERT_EQUAL(WifiStatus::WrongPassword, state.status());

    TEST_ASSERT_TRUE(state.request("Home", "right"));
    TEST_ASSERT_TRUE(state.takeRequest(ssid, password));
    TEST_ASSERT_EQUAL_STRING("right", password.c_str());
}

void test_status_names_are_the_pages(void)
{
    TEST_ASSERT_EQUAL_STRING("idle", wifiStatusName(WifiStatus::Idle));
    TEST_ASSERT_EQUAL_STRING("connecting", wifiStatusName(WifiStatus::Connecting));
    TEST_ASSERT_EQUAL_STRING("connected", wifiStatusName(WifiStatus::Connected));
    TEST_ASSERT_EQUAL_STRING("wrong_password", wifiStatusName(WifiStatus::WrongPassword));
    TEST_ASSERT_EQUAL_STRING("not_found", wifiStatusName(WifiStatus::NotFound));
    TEST_ASSERT_EQUAL_STRING("failed", wifiStatusName(WifiStatus::Failed));
}

// wifi_err_reason_t of ESP-IDF 4.4
void test_disconnect_reasons(void)
{
    TEST_ASSERT_EQUAL(WifiStatus::WrongPassword, wifiStatusForDisconnect(2));   // AUTH_EXPIRE
    TEST_ASSERT_EQUAL(WifiStatus::WrongPassword, wifiStatusForDisconnect(15));  // 4WAY_HANDSHAKE_TIMEOUT
    TEST_ASSERT_EQUAL(WifiStatus::WrongPassword, wifiStatusForDisconnect(202)); // AUTH_FAIL
    TEST_ASSERT_EQUAL(WifiStatus::WrongPassword, wifiStatusForDisconnect(204)); // HANDSHAKE_TIMEOUT
    TEST_ASSERT_EQUAL(WifiStatus::NotFound, wifiStatusForDisconnect(201));      // NO_AP_FOUND
    TEST_ASSERT_EQUAL(WifiStatus::Failed, wifiStatusForDisconnect(8));          // ASSOC_LEAVE
    TEST_ASSERT_EQUAL(WifiStatus::Failed, wifiStatusForDisconnect(205));        // CONNECTION_FAIL
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_starts_idle);
    RUN_TEST(test_a_request_is_connecting_at_once);
    RUN_TEST(test_the_loop_takes_a_request_once);
    RUN_TEST(test_no_new_request_while_connecting);
    RUN_TEST(test_after_a_failure_the_page_can_try_again);
    RUN_TEST(test_status_names_are_the_pages);
    RUN_TEST(test_disconnect_reasons);
    return UNITY_END();
}
