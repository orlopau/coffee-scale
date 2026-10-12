#include <stdio.h>
#include <string.h>

#include <algorithm>

#include "network/setup_web.h"

namespace SetupWeb
{
    static int hexValue(char c)
    {
        if (c >= '0' && c <= '9')
        {
            return c - '0';
        }
        if (c >= 'a' && c <= 'f')
        {
            return c - 'a' + 10;
        }
        if (c >= 'A' && c <= 'F')
        {
            return c - 'A' + 10;
        }
        return -1;
    }

    /** Decodes [begin, end) into value. */
    static bool decode(const char *begin, const char *end, std::string &value, size_t maxLength)
    {
        value.clear();
        for (const char *p = begin; p < end; p++)
        {
            if (*p == '+')
            {
                value += ' ';
            }
            else if (*p == '%')
            {
                if (end - p < 3)
                {
                    return false;
                }
                const int high = hexValue(p[1]);
                const int low = hexValue(p[2]);
                if (high < 0 || low < 0)
                {
                    return false;
                }
                value += (char)(high << 4 | low);
                p += 2;
            }
            else
            {
                value += *p;
            }
            if (value.size() > maxLength)
            {
                return false;
            }
        }
        return true;
    }

    bool formField(const char *body, size_t length, const char *key, std::string &value, size_t maxLength)
    {
        const size_t keyLength = strlen(key);
        const char *end = body + length;
        const char *field = body;
        while (field < end)
        {
            const char *fieldEnd = std::find(field, end, '&');
            const char *equals = std::find(field, fieldEnd, '=');
            if (equals < fieldEnd && (size_t)(equals - field) == keyLength && memcmp(field, key, keyLength) == 0)
            {
                return decode(equals + 1, fieldEnd, value, maxLength);
            }
            if (fieldEnd == end)
            {
                break;
            }
            field = fieldEnd + 1;
        }
        return false;
    }

    std::string jsonString(const std::string &text)
    {
        std::string json = "\"";
        for (unsigned char c : text)
        {
            if (c == '"' || c == '\\')
            {
                json += '\\';
                json += (char)c;
            }
            else if (c < 0x20)
            {
                char escaped[8];
                snprintf(escaped, sizeof(escaped), "\\u%04x", c);
                json += escaped;
            }
            else
            {
                json += (char)c;
            }
        }
        return json + "\"";
    }

    std::string networksJson(std::vector<Network> networks)
    {
        std::stable_sort(networks.begin(), networks.end(),
                         [](const Network &a, const Network &b) { return a.rssi > b.rssi; });
        std::vector<std::string> listed;
        std::string json = "[";
        for (const Network &network : networks)
        {
            if (network.ssid.empty() || std::find(listed.begin(), listed.end(), network.ssid) != listed.end())
            {
                continue;
            }
            listed.push_back(network.ssid);
            if (json.size() > 1)
            {
                json += ',';
            }
            json += "{\"ssid\":" + jsonString(network.ssid) + ",\"rssi\":" + std::to_string(network.rssi) +
                    ",\"secure\":" + (network.secure ? "true" : "false") + "}";
        }
        return json + "]";
    }
}
