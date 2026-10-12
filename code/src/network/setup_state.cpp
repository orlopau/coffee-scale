#include "network/setup_state.h"

const char *wifiStatusName(WifiStatus status)
{
    switch (status)
    {
    case WifiStatus::Idle:
        return "idle";
    case WifiStatus::Connecting:
        return "connecting";
    case WifiStatus::Connected:
        return "connected";
    case WifiStatus::WrongPassword:
        return "wrong_password";
    case WifiStatus::NotFound:
        return "not_found";
    case WifiStatus::Failed:
        return "failed";
    }
    return "failed";
}

WifiStatus wifiStatusForDisconnect(uint8_t reason)
{
    // wifi_err_reason_t of ESP-IDF 4.4, checked by a static_assert in wifi_station.cpp
    switch (reason)
    {
    case 2:   // AUTH_EXPIRE
    case 15:  // 4WAY_HANDSHAKE_TIMEOUT
    case 202: // AUTH_FAIL
    case 204: // HANDSHAKE_TIMEOUT
        return WifiStatus::WrongPassword;
    case 201: // NO_AP_FOUND
        return WifiStatus::NotFound;
    default:
        return WifiStatus::Failed;
    }
}

bool SetupState::request(const std::string &ssid, const std::string &password)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (current == WifiStatus::Connecting)
    {
        return false;
    }
    this->ssid = ssid;
    this->password = password;
    requested = true;
    current = WifiStatus::Connecting;
    return true;
}

bool SetupState::takeRequest(std::string &ssid, std::string &password)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (!requested)
    {
        return false;
    }
    ssid = this->ssid;
    password = this->password;
    requested = false;
    this->password.clear();
    return true;
}

void SetupState::finish(WifiStatus result)
{
    std::lock_guard<std::mutex> lock(mutex);
    current = result;
}

WifiStatus SetupState::status() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return current;
}
