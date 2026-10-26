#ifndef NATIVE

#pragma once

#include <IPAddress.h>

#include "weighing/sample.h"

/**
 * Records the load cell and saves the recordings to flash, with or without the
 * dev server's recording mode (see the README), entered from the updater.
 */
namespace Recording
{
    struct Server
    {
        IPAddress ip;
        uint16_t port;
        const char *path;
    };

    /** Records samples, saves them to flash and sends them to server, if there is one, until the scale is switched off. */
    [[noreturn]] void run(SampleSource &samples, const Server *server, float gramsPerCount);
}

#endif
