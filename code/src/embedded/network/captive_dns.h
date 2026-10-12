#ifndef NATIVE

#pragma once

#include <esp_netif.h>

/** Answers every DNS query on the setup network with the scale's address, in its own task. */
namespace CaptiveDns
{
    bool start(esp_ip4_addr_t ip);
    /** Stops the task and closes its socket; waits up to 1 s for that. */
    void stop();
}

#endif
