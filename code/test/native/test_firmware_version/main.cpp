#include <string.h>
#include <unity.h>

#include "firmware_version.h"

void setUp(void) {}

void tearDown(void) {}

void test_parses_with_and_without_v(void)
{
    FirmwareVersion::Parts parts;
    TEST_ASSERT_TRUE(FirmwareVersion::parse("v1.10.2", parts));
    TEST_ASSERT_EQUAL(1, parts.major);
    TEST_ASSERT_EQUAL(10, parts.minor);
    TEST_ASSERT_EQUAL(2, parts.patch);
    TEST_ASSERT_TRUE(FirmwareVersion::parse("0.0.0", parts));
    TEST_ASSERT_EQUAL(0, parts.major);
}

void test_rejects_anything_else(void)
{
    FirmwareVersion::Parts parts;
    const char *bad[] = {"", "v", "1.9", "v1.9.0.1", "v1.10.0-rc1", "v1..0", "x1.9.0", "V1.9.0",
                         "v1.9.0 ", "v99999.0.0", "v1.9.-1", "latest"};
    for (const char *text : bad)
    {
        TEST_ASSERT_FALSE_MESSAGE(FirmwareVersion::parse(text, parts), text);
    }
    TEST_ASSERT_FALSE(FirmwareVersion::parse(nullptr, parts));
}

void test_compares_by_number(void)
{
    TEST_ASSERT_TRUE(FirmwareVersion::isNewer("v1.10.0", "v1.9.0"));
    TEST_ASSERT_TRUE(FirmwareVersion::isNewer("v2.0.0", "v1.99.99"));
    TEST_ASSERT_TRUE(FirmwareVersion::isNewer("v1.9.1", "v1.9.0"));
    TEST_ASSERT_TRUE(FirmwareVersion::isNewer("1.9.1", "v1.9.0"));
    TEST_ASSERT_FALSE(FirmwareVersion::isNewer("v1.9.0", "v1.9.0"));
    TEST_ASSERT_FALSE(FirmwareVersion::isNewer("v1.9.0", "v1.10.0"));
    TEST_ASSERT_FALSE(FirmwareVersion::isNewer("v1.8.9", "v1.9.0"));
}

// e.g. a dev build without a tag, or nothing stored
void test_unreadable_is_never_newer(void)
{
    TEST_ASSERT_FALSE(FirmwareVersion::isNewer("", "v1.9.0"));
    TEST_ASSERT_FALSE(FirmwareVersion::isNewer("v1.10.0", "dev"));
    TEST_ASSERT_FALSE(FirmwareVersion::isNewer(nullptr, "v1.9.0"));
    TEST_ASSERT_FALSE(FirmwareVersion::isNewer("v1.10.0", nullptr));
}

void test_version_from_githubs_redirect(void)
{
    char version[FirmwareVersion::SIZE];
    TEST_ASSERT_TRUE(FirmwareVersion::fromRedirect(
        "https://github.com/orlopau/coffee-scale/releases/download/firmware-v1.10.0/firmware_en.bin", version,
        sizeof(version)));
    TEST_ASSERT_EQUAL_STRING("v1.10.0", version);
}

// the dev server's release, see scripts/dev_server.py --release
void test_version_from_a_relative_redirect(void)
{
    char version[FirmwareVersion::SIZE];
    TEST_ASSERT_TRUE(FirmwareVersion::fromRedirect("/releases/download/firmware-v9.9.9/firmware_de.bin", version,
                                                   sizeof(version)));
    TEST_ASSERT_EQUAL_STRING("v9.9.9", version);
}

// the repository has PCB releases too
void test_a_pcb_release_is_no_version(void)
{
    char version[FirmwareVersion::SIZE] = "untouched";
    TEST_ASSERT_FALSE(FirmwareVersion::fromRedirect(
        "https://github.com/orlopau/coffee-scale/releases/download/pcb-v1.3.0/firmware_en.bin", version,
        sizeof(version)));
    TEST_ASSERT_EQUAL_STRING("", version);
}

// a guest network that sends every request to its login page
void test_a_captive_portal_is_no_version(void)
{
    char version[FirmwareVersion::SIZE];
    TEST_ASSERT_FALSE(FirmwareVersion::fromRedirect("http://login.hotel.example/portal?orig=github.com", version,
                                                    sizeof(version)));
    TEST_ASSERT_EQUAL_STRING("", version);
}

void test_broken_redirects_are_no_version(void)
{
    char version[FirmwareVersion::SIZE];
    const char *bad[] = {
        "",
        "/releases/download/firmware-v1.10.0",            // no file after it
        "/releases/download/firmware-/firmware_en.bin",   // no version
        "/releases/download/firmware-latest/firmware_en.bin",
        "/releases/download/firmware-v1.10.0-rc1/firmware_en.bin",
        "/releases/download/firmware-v1234567890123.0.0/firmware_en.bin", // too many digits
    };
    for (const char *location : bad)
    {
        TEST_ASSERT_FALSE_MESSAGE(FirmwareVersion::fromRedirect(location, version, sizeof(version)), location);
        TEST_ASSERT_EQUAL_STRING_MESSAGE("", version, location);
    }
    TEST_ASSERT_FALSE(FirmwareVersion::fromRedirect(nullptr, version, sizeof(version)));
}

void test_a_version_that_doesnt_fit_is_none(void)
{
    char version[6];
    TEST_ASSERT_FALSE(
        FirmwareVersion::fromRedirect("/releases/download/firmware-v1.10.0/firmware_en.bin", version, sizeof(version)));
    TEST_ASSERT_EQUAL_STRING("", version);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_parses_with_and_without_v);
    RUN_TEST(test_rejects_anything_else);
    RUN_TEST(test_compares_by_number);
    RUN_TEST(test_unreadable_is_never_newer);
    RUN_TEST(test_version_from_githubs_redirect);
    RUN_TEST(test_version_from_a_relative_redirect);
    RUN_TEST(test_a_pcb_release_is_no_version);
    RUN_TEST(test_a_captive_portal_is_no_version);
    RUN_TEST(test_broken_redirects_are_no_version);
    RUN_TEST(test_a_version_that_doesnt_fit_is_none);
    return UNITY_END();
}
