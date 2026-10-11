#ifndef NATIVE

#pragma once

#include <IPAddress.h>

#include "weighing/sample.h"

/**
 * Records the load cell for the dev server's recording mode (see the README),
 * entered from the updater.
 */
namespace Recording
{
    /** Records samples and uploads to http://server:port/path until the scale is switched off. */
    [[noreturn]] void run(SampleSource &samples, IPAddress server, uint16_t port, const char *path, float gramsPerCount);
}

#endif
