#ifndef NATIVE

#include <AutoConnectCore.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#include "constants.h"
#include "display.h"
#include "interface.h"
#include "data/localization.h"
#include "ui/updater_screens.h"
#include "embedded/record.h"

#define TAG "UPDATER"

// mDNS service announced by scripts/dev_server.py (`pio run -t serve` or `-t record`)
#define DEV_SERVER_SERVICE "coffeescale-fw"
// each attempt waits up to 3 s for answers
#define DEV_SERVER_SEARCH_ATTEMPTS 3

namespace Updater
{
    AutoConnect portal;

    // calibration of the scale, written into recordings
    static float gramsPerCount = 1;

    static void updateFromDevServer();
    static void runUpdate(WiFiClient &client, const char *url);

    // The updater blocks the main loop, so it draws and sends frames itself.

    static void showMessage(const char *text)
    {
        Canvas &canvas = Display::canvas();
        canvas.clear();
        UpdaterScreens::message(canvas, text);
        canvas.flush();
    }

    static void showText(const char *text)
    {
        Canvas &canvas = Display::canvas();
        canvas.clear();
        UpdaterScreens::text(canvas, text);
        canvas.flush();
    }

    static void showSwitcher(const char *title, uint8_t index, uint8_t count, const char *const options[])
    {
        Canvas &canvas = Display::canvas();
        canvas.clear();
        UpdaterScreens::switcher(canvas, title, index, count, options);
        canvas.flush();
    }

    void started() { ESP_LOGI(TAG, "CALLBACK:  HTTP update process started"); }

    void finished() { ESP_LOGI(TAG, "CALLBACK:  HTTP update process finished"); }

    void progress(int cur, int total)
    {
        ESP_LOGI(TAG, "CALLBACK:  HTTP update process at %d of %d bytes...", cur, total);
        static char progress[32];
        sprintf(progress, UPDATER_PROGRESS, ((float)cur / (float)total) * 100);
        showMessage(progress);
    }

    void error(int err) { ESP_LOGI(TAG, "CALLBACK:  HTTP update fatal error code %d", err); }

    bool onCaptivePortalStart(IPAddress &address)
    {
        ESP_LOGI(TAG, "Captive portal started");
        static char text[64];
        sprintf(text, UPDATER_WIFI_CONNECT_MANUAL, WiFi.softAPSSID().c_str());
        showText(text);
        return true;
    }

    void update_firmware(float scale)
    {
        gramsPerCount = scale;
        showMessage(UPDATER_UPDATING);
        ESP_LOGI(TAG, "Updating firmware...");

        uint32_t id = 0;
        for (int i = 0; i < 17; i = i + 8)
        {
            id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
        }

        char ssid[25];
        sprintf(ssid, "CoffeeScale-%d", id);
        AutoConnectConfig config(ssid, "");
        config.title = "CoffeeScale";
        config.boundaryOffset = 1024;
        portal.config(config);

        portal.onDetect(onCaptivePortalStart);

        if (portal.begin())
        {
            ESP_LOGI(TAG, "WiFi connected");
            showMessage(UPDATER_WIFI_CONNECTED);
        }

        delay(1000);

        // The button may still be held from entering the updater at boot. Wait
        // until it is released, so holding it does not count as a long press.
        while (Interface::isEncoderPressed())
        {
            Interface::update();
            delay(10);
        }
        Interface::consumeEncoderClick();

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

            selectedQualifier += static_cast<int>(Interface::getEncoderDirection());
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

        WiFiClientSecure client;
        client.setInsecure();

        char url[128];
        snprintf(url, 128, UPDATE_URL, qualifiers[selectedQualifier]);
        runUpdate(client, url);
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
            Recording::run(server.ip, server.port, server.recordPath.c_str(), gramsPerCount);
        }

        char url[128];
        snprintf(url, sizeof(url), "http://%s:%u%s", server.ip.toString().c_str(), server.port, server.firmwarePath.c_str());
        WiFiClient client;
        runUpdate(client, url);
    }

    static void runUpdate(WiFiClient &client, const char *url)
    {
        HTTPUpdate httpUpdate;
        httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
        httpUpdate.onStart(started);
        httpUpdate.onEnd(finished);
        httpUpdate.onProgress(progress);
        httpUpdate.onError(error);

        ESP_LOGI(TAG, "Update URL: %s", url);
        t_httpUpdate_return code = httpUpdate.update(client, url);

        switch (code)
        {
        case HTTP_UPDATE_FAILED:
            ESP_LOGI(TAG, "HTTP_UPDATE_FAILED Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            showMessage(UPDATER_FAILED);
            break;

        case HTTP_UPDATE_NO_UPDATES:
            ESP_LOGI(TAG, "HTTP_UPDATE_NO_UPDATES");
            showMessage(UPDATER_NO_UPDATE);
            break;

        case HTTP_UPDATE_OK:
            ESP_LOGI(TAG, "HTTP_UPDATE_OK");
            showMessage(UPDATER_SUCCESS);
            break;
        }

        for (;;)
        {
            delay(1000);
        }
    }
}

#endif