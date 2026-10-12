#ifndef NATIVE

#include <Arduino.h>
#include <ESPmDNS.h>

#include "constants.h"
#include "display.h"
#include "interface.h"
#include "data/localization.h"
#include "ui/updater_screens.h"
#include "embedded/record.h"
#include "embedded/network/firmware_check.h"
#include "embedded/network/firmware_download.h"
#include "embedded/network/wifi_setup.h"
#include "embedded/network/wifi_station.h"

#define TAG "UPDATER"

// mDNS service announced by scripts/dev_server.py (`pio run -t serve` or `-t record`)
#define DEV_SERVER_SERVICE "coffeescale-fw"
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

    static void showMessage(const char *text)
    {
        Canvas &canvas = Display::canvas();
        canvas.clear();
        UpdaterScreens::message(canvas, text);
        canvas.flush();
    }

    static void showSwitcher(const char *title, uint8_t index, uint8_t count, const char *const options[])
    {
        Canvas &canvas = Display::canvas();
        canvas.clear();
        UpdaterScreens::switcher(canvas, title, index, count, options);
        canvas.flush();
    }

    static void progress(size_t done, size_t total)
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

    /** The saved network, or the setup if there is none or it can't be reached. Stays here if neither works. */
    static void connect()
    {
        WifiStatus status = WifiStatus::Failed;
        if (WifiStation::begin() && WifiStation::hasSavedNetwork())
        {
            status = WifiStation::connect(SAVED_NETWORK_TIMEOUT_MS);
        }
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

        connect();

        delay(1000);

        // The button may still be held from entering the updater at boot. Wait
        // until it is released, so holding it does not count as a long press.
        while (Interface::isEncoderPressed())
        {
            Interface::update();
            delay(10);
        }
        Interface::discardClicks();

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
                updateFromDevServer(); // only returns if no server was found
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

    struct DevServer
    {
        IPAddress ip;
        uint16_t port;
        /** Path of the firmware, if the server serves one (`pio run -t serve`). */
        String firmwarePath;
        /** Path to upload recordings to, if the server receives them (`pio run -t record`). */
        String recordPath;
    };

    static bool findDevServer(DevServer &server)
    {
        if (!MDNS.begin("coffee-scale"))
        {
            ESP_LOGE(TAG, "mDNS could not be started");
            return false;
        }

        for (int attempt = 0; attempt < DEV_SERVER_SEARCH_ATTEMPTS; attempt++)
        {
            int found = MDNS.queryService(DEV_SERVER_SERVICE, "tcp");
            if (found > 0)
            {
                server.ip = MDNS.IP(0);
                server.port = MDNS.port(0);
                server.firmwarePath = MDNS.txt(0, "path");
                server.recordPath = MDNS.txt(0, "record");
                if (server.firmwarePath.length() == 0 && server.recordPath.length() == 0)
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

        DevServer server;
        if (!findDevServer(server))
        {
            ESP_LOGI(TAG, "No dev firmware server found");
            showMessage(UPDATER_DEV_NOT_FOUND);
            delay(2000);
            return;
        }

        if (server.recordPath.length() > 0)
        {
            Recording::run(*loadCellSamples, server.ip, server.port, server.recordPath.c_str(), gramsPerCount);
        }

        char url[128];
        snprintf(url, sizeof(url), "http://%s:%u%s", server.ip.toString().c_str(), server.port, server.firmwarePath.c_str());
        runUpdate(url);
    }

    static void runUpdate(const char *url)
    {
        ESP_LOGI(TAG, "Update URL: %s", url);
        switch (FirmwareDownload::run(url, progress, ESP.getSketchMD5().c_str()))
        {
        case FirmwareDownload::Result::Installed:
            ESP_LOGI(TAG, "Update installed");
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