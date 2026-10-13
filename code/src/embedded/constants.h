#ifndef NATIVE

#pragma once

// Pin definitions
#define PIN_BOOT 0

#define PIN_I2C_SDA 22
#define PIN_I2C_SCL 23

#define PIN_BUZZER 16

#define PIN_BAT_CHARGE_STATUS 32

#define PIN_HX711_SCK 25
#define PIN_HX711_DAT 26

#define PIN_ENC_A 18
#define PIN_ENC_B 19
#define PIN_ENC_BTN 17

#define PIN_BAT_ADC 36

// Other constants
#define BAT_V_MULTIPLIER 2.0f
#define BAT_V_MIN 3.2f
#define BAT_V_MAX 4.2f

#define ADC_OFFSET 0.04f

// BUTTONS
#define PIN_UPDATE_FIRMWARE PIN_ENC_BTN

// UPDATE
// RELEASES_URL=http://<computer>:<port>/releases at build time points the scale at the dev server's release, see the README
#ifndef RELEASES_URL
#define RELEASES_URL "https://github.com/orlopau/coffee-scale/releases"
#endif
#define UPDATE_URL RELEASES_URL "/latest/download/firmware%s.bin"

// the release's firmware in the language of this one
#ifdef LANG_DE
#define FIRMWARE_FILE "firmware_de.bin"
#else
#define FIRMWARE_FILE "firmware_en.bin"
#endif
// redirects to the latest release, whose version is in the redirect
#define LATEST_RELEASE_URL RELEASES_URL "/latest/download/" FIRMWARE_FILE
// one version's firmware, %s is the version
#define RELEASE_URL RELEASES_URL "/download/firmware-%s/" FIRMWARE_FILE

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "0.0.0"
#endif

#endif
