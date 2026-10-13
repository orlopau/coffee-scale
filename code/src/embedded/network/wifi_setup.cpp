#ifndef NATIVE

#include <Arduino.h>
#include <esp_log.h>
#include <esp_mac.h>

#include <string>

#include "interface.h"
#include "settings.h"
#include "data/localization.h"
#include "embedded/blocking_screens.h"
#include "embedded/network/captive_dns.h"
#include "embedded/network/firmware_check.h"
#include "embedded/network/setup_portal.h"
#include "embedded/network/wifi_setup.h"
#include "embedded/network/wifi_station.h"
#include "network/setup_network.h"
#include "ui/wifi_screens.h"

#define TAG "WIFI_SETUP"

namespace WifiSetup
{
    // trying a network takes up to 3 attempts within this
    static const uint32_t CONNECT_TIMEOUT_MS = 15000;
    // the phone gets to see "connected" before the setup network closes
    static const uint32_t CONNECTED_SHOWN_MS = 5000;
    static const uint32_t FAILURE_SHOWN_MS = 3000;

    // The setup blocks the main loop, so it draws and sends frames itself.

    static void showJoin(const char *payload, const char *name)
    {
        BlockingScreens::show(
            [&](Canvas &canvas) { WifiScreens::join(canvas, payload, name, SetupNetwork::PASSWORD); });
    }

    static void showStatus(const char *text)
    {
        BlockingScreens::show([text](Canvas &canvas) { WifiScreens::status(canvas, text); });
    }

    static const char *failureText(WifiStatus status)
    {
        switch (status)
        {
        case WifiStatus::WrongPassword:
            return WIFI_WRONG_PASSWORD;
        case WifiStatus::NotFound:
            return WIFI_NOT_FOUND;
        default:
            return WIFI_FAILED;
        }
    }

    static void close()
    {
        SetupPortal::stop();
        CaptiveDns::stop();
        WifiStation::stopAccessPoint();
    }

    WifiStatus run()
    {
        uint8_t mac[6];
        esp_efuse_mac_get_default(mac);
        char name[24];
        char payload[64];
        SetupNetwork::name(mac, name, sizeof(name));
        SetupNetwork::qrPayload(name, payload, sizeof(payload));

        // the check for updates may be online meanwhile, it keeps its connection until a network is tried
        WifiStation::Radio radio;

        SetupState state;
        if (!WifiStation::begin() || !WifiStation::startAccessPoint(name, SetupNetwork::PASSWORD) ||
            !CaptiveDns::start(WifiStation::accessPointIp()) || !SetupPortal::start(state))
        {
            close();
            showStatus(WIFI_UNAVAILABLE);
            delay(FAILURE_SHOWN_MS);
            return WifiStatus::Failed;
        }
        ESP_LOGI(TAG, "Setup network %s is open", name);
        // a press still held, e.g. from entering the updater at boot, must not cancel
        BlockingScreens::waitForRelease();
        showJoin(payload, name);

        WifiStatus result = WifiStatus::Idle;
        for (;;)
        {
            Interface::update();
            std::string ssid, password;
            if (state.takeRequest(ssid, password))
            {
                ESP_LOGI(TAG, "Trying %s", ssid.c_str());
                showStatus(WIFI_CONNECTING);
                const WifiStatus status = WifiStation::tryNetwork(ssid.c_str(), password.c_str(), CONNECT_TIMEOUT_MS);
                state.finish(status);
                if (status == WifiStatus::Connected)
                {
                    showStatus(WIFI_CONNECTED);
                    delay(CONNECTED_SHOWN_MS);
                    result = status;
                    break;
                }
                showStatus(failureText(status));
                delay(FAILURE_SHOWN_MS);
                // a click while the failure showed must not cancel the next try
                Interface::discardClicks();
                showJoin(payload, name);
            }
            else if (Interface::getEncoderClick() == ClickType::SINGLE)
            {
                ESP_LOGI(TAG, "Setup cancelled");
                break;
            }
            delay(20);
        }

        close();
        Interface::discardClicks();
        return result;
    }

    void askOnce()
    {
        if (Settings::getFlag(Settings::WIFI_ASKED_KEY) || FirmwareCheck::onProbation())
        {
            return;
        }
        const uint8_t index = BlockingScreens::choose(2, WifiScreens::ask);
        Settings::setFlag(Settings::WIFI_ASKED_KEY, true);
        if (index == 0)
        {
            run();
        }
    }
}

#endif
