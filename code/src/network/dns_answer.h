#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * The captive portal's DNS: every name resolves to the scale, so phones that
 * check for internet land on the setup page.
 */
namespace DnsAnswer
{
    /**
     * Writes the reply to a DNS query into out: for an A query the address ip
     * (4 bytes in order, e.g. {192, 168, 4, 1}), for any other type no
     * answer. Returns the reply's length, 0 if query isn't a standard query
     * with one question or the reply doesn't fit into outSize.
     */
    size_t reply(const uint8_t *query, size_t length, const uint8_t ip[4], uint8_t *out, size_t outSize);
}
