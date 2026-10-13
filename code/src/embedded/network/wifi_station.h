#ifndef NATIVE

#pragma once

#include <stdint.h>

#include <vector>

#include <esp_netif.h>

#include "network/setup_state.h"
#include "network/setup_web.h"

/**
 * The scale's Wi-Fi, on the ESP-IDF API. ESP-IDF keeps the one saved network
 * in NVS, so a network AutoConnect saved is found there too.
 */
namespace WifiStation
{
    /** Starts the network stack and the Wi-Fi driver, once. False, and logged, if that fails. */
    bool begin();
    /**
     * Shuts the Wi-Fi driver down again to free its memory, if nobody keeps
     * Wi-Fi on; begin() starts it again. Call from the loop's task.
     */
    void end();
    bool hasSavedNetwork();
    /**
     * Connects to ssid for up to timeoutMs, and saves it only if that worked.
     * Whoever was online through the station before loses that connection.
     */
    WifiStatus tryNetwork(const char *ssid, const char *password, uint32_t timeoutMs);
    /** The networks in range, empty if scanning fails, e.g. while connecting. Takes about 2 s. */
    std::vector<SetupWeb::Network> scan();
    /** Opens the setup's own WPA2 network next to the station. Phones get the scale as their DNS server. */
    bool startAccessPoint(const char *ssid, const char *password);
    /** Closes the setup's own network; the station stays as it is. */
    void stopAccessPoint();
    /** The setup network's address, 4.3.2.1. */
    esp_ip4_addr_t accessPointIp();

    /**
     * Keeps Wi-Fi on while it lives; the last one to end turns it off to save
     * the battery, the saved network stays. Whoever needs Wi-Fi holds one,
     * e.g. the setup; nobody waits for another one.
     */
    class Radio
    {
    public:
        Radio();
        ~Radio();
        Radio(const Radio &) = delete;
        Radio &operator=(const Radio &) = delete;
    };

    /**
     * Online through the saved network while it lives: shares the station's
     * connection if it has one, otherwise connects, for up to timeoutMs. A
     * second one while one connects waits for that attempt. Keeps Wi-Fi on
     * like a Radio. Calls begin(), whose first call must be in the loop's task.
     */
    class Online
    {
    public:
        explicit Online(uint32_t timeoutMs);
        bool connected() const { return status == WifiStatus::Connected; }

    private:
        Radio radio;
        WifiStatus status = WifiStatus::Failed;
    };
}

#endif
