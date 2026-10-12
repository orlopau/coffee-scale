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
    bool hasSavedNetwork();
    /** Connects to the saved network, for up to timeoutMs. */
    WifiStatus connect(uint32_t timeoutMs);
    /** Connects to ssid for up to timeoutMs, and saves it only if that worked. */
    WifiStatus tryNetwork(const char *ssid, const char *password, uint32_t timeoutMs);
    /** The networks in range, empty if scanning fails, e.g. while connecting. Takes about 2 s. */
    std::vector<SetupWeb::Network> scan();
    /** Opens the setup's own WPA2 network next to the station. Phones get the scale as their DNS server. */
    bool startAccessPoint(const char *ssid, const char *password);
    /** Closes the setup's own network; the station stays as it is. */
    void stopAccessPoint();
    /** The setup network's address, 4.3.2.1. */
    esp_ip4_addr_t accessPointIp();
    /** Turns Wi-Fi off, to save the battery. The saved network stays. */
    void stop();
}

#endif
