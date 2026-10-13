#ifndef NATIVE

#pragma once

#include "network/setup_state.h"

/**
 * The Wi-Fi setup from the phone: the scale opens its own network and shows
 * its QR code, the phone joins it, and the setup page opens by itself.
 */
namespace WifiSetup
{
    /**
     * Runs until the scale is connected (Connected) or a click cancels
     * (Idle). Failed if Wi-Fi doesn't start. Wi-Fi goes off when done unless
     * someone else keeps it on, like the updater with its WifiStation::Online.
     * Blocks and draws its own screens.
     */
    WifiStatus run();

    /**
     * Asks once whether to set up Wi-Fi now, and runs the setup on "Yes".
     * Waits while a fresh update is on probation: switching off at the
     * question would roll back a good firmware.
     */
    void askOnce();
}

#endif
