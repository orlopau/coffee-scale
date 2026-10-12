#include <string.h>
#include <unity.h>

#include "network/setup_network.h"

void setUp(void) {}

void tearDown(void) {}

// the last three bytes of the MAC as one number, like AutoConnect named it
void test_name_is_the_macs_last_three_bytes(void)
{
    const uint8_t mac[6] = {0x24, 0x0A, 0xC4, 0x12, 0x34, 0x56};
    char name[24];
    SetupNetwork::name(mac, name, sizeof(name));
    TEST_ASSERT_EQUAL_STRING("CoffeeScale-1193046", name);
}

void test_qr_payload_joins_with_the_password(void)
{
    char payload[64];
    SetupNetwork::qrPayload("CoffeeScale-1193046", payload, sizeof(payload));
    TEST_ASSERT_EQUAL_STRING("WIFI:T:WPA;S:CoffeeScale-1193046;P:coffee-scale;;", payload);
}

// QR version 3 holds 53 bytes at low error correction, see WifiScreens::join()
void test_longest_payload_fits_qr_version_3(void)
{
    const uint8_t mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    char name[24];
    char payload[64];
    SetupNetwork::name(mac, name, sizeof(name));
    SetupNetwork::qrPayload(name, payload, sizeof(payload));
    TEST_ASSERT_EQUAL_STRING("CoffeeScale-16777215", name);
    TEST_ASSERT_LESS_OR_EQUAL(53, strlen(payload));
}

// WPA2 needs 8 to 63 characters
void test_password_suits_wpa2(void)
{
    TEST_ASSERT_GREATER_OR_EQUAL(8, strlen(SetupNetwork::PASSWORD));
    TEST_ASSERT_LESS_OR_EQUAL(63, strlen(SetupNetwork::PASSWORD));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_name_is_the_macs_last_three_bytes);
    RUN_TEST(test_qr_payload_joins_with_the_password);
    RUN_TEST(test_longest_payload_fits_qr_version_3);
    RUN_TEST(test_password_suits_wpa2);
    return UNITY_END();
}
