#ifndef NATIVE

#pragma once

#include <stdint.h>

#include <string>

#include <esp_netif_ip_addr.h>

/**
 * Finds the dev server scripts/dev_server.py announces on the laptop, for the
 * updater and the log. Both search through here: the ESP-IDF mDNS API returns
 * its results to the caller, unlike Arduino's MDNS with its shared state.
 */
namespace DevServerSearch
{
    struct Server
    {
        esp_ip4_addr_t ip;
        uint16_t port;            // HTTP
        std::string firmwarePath; // TXT "path", empty if not offered
        std::string recordPath;   // TXT "record", empty if not offered
        uint16_t logPort;         // TXT "log", 0 if not offered
    };

    /**
     * Asks mDNS once for _coffeescale-fw._tcp, waiting up to timeoutMs. Starts mDNS (hostname
     * "coffee-scale") the first time, from any task; false if that or the query fails.
     */
    bool find(Server &server, uint32_t timeoutMs);
}

#endif
