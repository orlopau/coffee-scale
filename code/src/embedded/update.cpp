#ifndef NATIVE

#include <AutoConnectCore.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WebServer.h>
#include <WiFi.h>

#include "constants.h"
#include "user_input.h"

namespace Updater
{
    AutoConnect portal;

    void started() { Serial.println("CALLBACK:  HTTP update process started"); }

    void finished() { Serial.println("CALLBACK:  HTTP update process finished"); }

    void progress(Display &display, int cur, int total)
    {
        Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
        static char progress[32];
        sprintf(progress, "Updating: %.2f%%", ((float)cur / (float)total) * 100);
        display.centerText(progress, 13);
    }

    void error(int err) { Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err); }

    bool onCaptivePortalStart(Display &display, IPAddress &address)
    {
        Serial.println("Captive portal started");
        static char text[64];
        sprintf(text, "Setup WiFi by\nconnecting to the\nnetwork:\n%s\nand clicking\n\"Configure new AP\".", WiFi.softAPSSID().c_str());
        display.text(text);
        return true;
    }

    void update_firmware(Display &display)
    {
        display.centerText("Updating...", 13);
        Serial.println("Updating firmware...");

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

        auto captivePortalStartFunc = std::bind(onCaptivePortalStart, std::ref(display), std::placeholders::_1);
        portal.onDetect(captivePortalStartFunc);

        if (portal.begin())
        {
            Serial.println("WiFi connected");
            display.centerText("WiFi connected.", 13);
        }

        delay(2000);

        HTTPUpdate httpUpdate;
        httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
        httpUpdate.onStart(started);
        httpUpdate.onEnd(finished);
        auto progressFunc = std::bind(progress, std::ref(display), std::placeholders::_1, std::placeholders::_2);
        httpUpdate.onProgress(progressFunc);
        httpUpdate.onError(error);
        
        WiFiClientSecure client;
        client.setInsecure();
        t_httpUpdate_return code = httpUpdate.update(client, UPDATE_URL);

        switch (code)
        {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            display.centerText("Update failed.", 13);
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            display.centerText("No updates.", 13);
            break;

        case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            display.centerText("Update successful.", 13);
            break;
        }

        for (;;)
        {
            delay(1000);
        }
    }
}

#endif