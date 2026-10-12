#include <stdio.h>

#include "network/setup_network.h"

namespace SetupNetwork
{
    void name(const uint8_t mac[6], char *out, size_t size)
    {
        uint32_t id = (uint32_t)mac[3] << 16 | (uint32_t)mac[4] << 8 | mac[5];
        snprintf(out, size, "CoffeeScale-%lu", (unsigned long)id);
    }

    void qrPayload(const char *name, char *out, size_t size)
    {
        // neither the name nor the password has one of the characters the format escapes: \ ; , : "
        snprintf(out, size, "WIFI:T:WPA;S:%s;P:%s;;", name, PASSWORD);
    }
}
