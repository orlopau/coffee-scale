#ifndef NATIVE

#include <Arduino.h>

#include "constants.h"
#include "interface.h"
#include "data/localization.h"
#include "ui/updater_screens.h"
#include "embedded/blocking_screens.h"
#include "embedded/record.h"
#include "embedded/network/auto_update.h"
#include "embedded/network/dev_server_search.h"
#include "embedded/network/firmware_check.h"
#include "embedded/network/firmware_download.h"
#include "embedded/network/wifi_setup.h"
#include "embedded/network/wifi_station.h"

#define TAG "UPDATER"

// The mDNS service scripts/dev_server.py announces (`pio run -t serve` or `-t record`) is
// searched by DevServerSearch, also for the log, see net_log.h.
// each attempt waits up to 3 s for answers
#define DEV_SERVER_SEARCH_ATTEMPTS 3
// how long the updater tries the saved network before it runs the setup
#define SAVED_NETWORK_TIMEOUT_MS 15000

namespace Updater
{
    // calibration of the scale, written into recordings
    static float gramsPerCount = 1;
    // the load cell's readings, for recordings
    static SampleSource *loadCellSamples = nullptr;

    static void updateFromDevServer();
    static void runUpdate(const char *url);

    // The updater blocks the main loop, so it draws and sends frames itself.

    void showMessage(const char *text)
    {
        BlockingScreens::show([text](Canvas &canvas) { UpdaterScreens::message(canvas, text); });
    }

    static void showSwitcher(const char *title, uint8_t index, uint8_t count, const char *const options[])
    {
        BlockingScreens::show(
            [&](Canvas &canvas) { UpdaterScreens::switcher(canvas, title, index, count, options); });
    }

    void showProgress(size_t done, size_t total)
    {
        static char text[32];
        if (total > 0)
        {
            snprintf(text, sizeof(text), UPDATER_PROGRESS, 100.0f * done / total);
        }
        else
        {
            snprintf(text, sizeof(text), "%u kB", (unsigned)(done / 1024));
        }
        showMessage(text);
    }

    /** online, or the setup if there is no saved network or it can't be reached. Stays here if neither works. */
    static void connect(const WifiStation::Online &online)
    {
        WifiStatus status = online.connected() ? WifiStatus::Connected : WifiStatus::Failed;
        if (status != WifiStatus::Connected)
        {
            ESP_LOGI(TAG, "No saved network reachable, running the setup");
            status = WifiSetup::run();
        }
        if (status != WifiStatus::Connected)
        {
            showMessage(UPDATER_FAILED);
            for (;;)
            {
                delay(1000);
            }
        }
        ESP_LOGI(TAG, "WiFi connected");
        showMessage(UPDATER_WIFI_CONNECTED);
    }

    void update_firmware(float scale, SampleSource &samples)
    {
        gramsPerCount = scale;
        loadCellSamples = &samples;
        showMessage(UPDATER_UPDATING);
        ESP_LOGI(TAG, "Updating firmware...");

        // keeps Wi-Fi on until the restart: the updater doesn't return
        WifiStation::Online online(SAVED_NETWORK_TIMEOUT_MS);
        connect(online);

        delay(1000);

        // The button may still be held from entering the updater at boot. Wait
        // until it is released, so holding it does not count as a long press.
        BlockingScreens::waitForRelease();

        // choose firmware type
        int selectedQualifier = 0;
        const uint8_t numQualifiers = 2;
        const char *names[] = {"Deutsch", "English"};
        const char *qualifiers[] = {"_de", "_en"};
        for (;;)
        {
            ClickType click = Interface::getEncoderClick();
            if (click == ClickType::SINGLE)
            {
                break;
            }
            // hidden: long press updates from a developer's computer instead of GitHub
            if (click == ClickType::LONG)
            {
                Interface::consumeEncoderClick();
                updateFromDevServer(); // records or installs, then restarts or waits for the switch-off
            }

            selectedQualifier += Interface::getEncoderTurns();
            if (selectedQualifier < 0)
            {
                selectedQualifier = 0;
            }
            else if (selectedQualifier >= numQualifiers)
            {
                selectedQualifier = numQualifiers - 1;
            }
            showSwitcher("Choose Language", selectedQualifier, numQualifiers, names);
            Interface::update();
        }

        char url[128];
        snprintf(url, sizeof(url), UPDATE_URL, qualifiers[selectedQualifier]);
        runUpdate(url);
    }

    static bool findDevServer(DevServerSearch::Server &server)
    {
        for (int attempt = 0; attempt < DEV_SERVER_SEARCH_ATTEMPTS; attempt++)
        {
            if (DevServerSearch::find(server, 3000))
            {
                if (server.firmwarePath.empty() && server.recordPath.empty())
                {
                    server.firmwarePath = "/firmware.bin";
                }
                return true;
            }
        }
        return false;
    }

    static void updateFromDevServer()
    {
        showMessage(UPDATER_DEV_SEARCHING);
        ESP_LOGI(TAG, "Searching for dev firmware server...");

        DevServerSearch::Server server;
        if (!findDevServer(server))
        {
            ESP_LOGI(TAG, "No dev firmware server found, recording without one");
            Recording::run(*loadCellSamples, nullptr, gramsPerCount);
        }

        if (!server.recordPath.empty())
        {
            Recording::Server recordServer{IPAddress(server.ip.addr), server.port, server.recordPath.c_str()};
            Recording::run(*loadCellSamples, &recordServer, gramsPerCount);
        }

        char url[128];
        snprintf(url, sizeof(url), "http://%s:%u%s", IPAddress(server.ip.addr).toString().c_str(), server.port, server.firmwarePath.c_str());
        runUpdate(url);
    }

    static void runUpdate(const char *url)
    {
        ESP_LOGI(TAG, "Update URL: %s", url);
        switch (FirmwareDownload::run(url, showProgress, ESP.getSketchMD5().c_str()))
        {
        case FirmwareDownload::Result::Installed:
            ESP_LOGI(TAG, "Update installed");
            // installed by hand: a rollback of it isn't a release to skip
            AutoUpdate::clearInstalling();
            // so the next start says so if this firmware gets rolled back
            FirmwareCheck::expectNewFirmware();
            showMessage(UPDATER_SUCCESS);
            delay(2000);
            esp_restart();
            break;
        case FirmwareDownload::Result::NoUpdate:
            showMessage(UPDATER_NO_UPDATE);
            break;
        case FirmwareDownload::Result::Failed:
            showMessage(UPDATER_FAILED);
            break;
        }

        for (;;)
        {
            delay(1000);
        }
    }
}

#endif