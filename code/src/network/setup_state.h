#pragma once

#include <stdint.h>

#include <mutex>
#include <string>

/** Where the Wi-Fi setup is, and how connecting to a network ended. */
enum class WifiStatus
{
    Idle,
    Connecting,
    Connected,
    WrongPassword,
    NotFound,
    Failed,
};

/** The status as the setup page names it, e.g. "wrong_password". */
const char *wifiStatusName(WifiStatus status);

/** How a connection ended that the station lost with this ESP-IDF disconnect reason (wifi_err_reason_t). */
WifiStatus wifiStatusForDisconnect(uint8_t reason);

/**
 * Hands the network entered on the page from the HTTP server's task to the
 * setup's loop, and the result back. Safe to use from both tasks.
 */
class SetupState
{
public:
    /** From the page: connect to ssid. False while a network is being tried. */
    bool request(const std::string &ssid, const std::string &password);
    /** From the setup's loop: the network to try, if one was requested since the last call. */
    bool takeRequest(std::string &ssid, std::string &password);
    /** From the setup's loop: how trying the network ended. */
    void finish(WifiStatus result);
    WifiStatus status() const;

private:
    mutable std::mutex mutex;
    WifiStatus current = WifiStatus::Idle;
    bool requested = false;
    std::string ssid;
    std::string password;
};
