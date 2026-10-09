#ifndef NATIVE

#pragma once

#include <IPAddress.h>

/**
 * Records the load cell for the dev server's recording mode (see the README),
 * entered from the updater.
 */
namespace Recording
{
    /** Records and uploads to http://server:port/path until the scale is switched off. */
    [[noreturn]] void run(IPAddress server, uint16_t port, const char *path, float gramsPerCount);
}

#endif
