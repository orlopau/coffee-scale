#include <string.h>

#include "network/dns_answer.h"

namespace DnsAnswer
{
    static const size_t HEADER = 12;
    static const uint16_t TYPE_A = 1;
    static const uint16_t CLASS_IN = 1;
    // seconds a phone may keep the answer, short since the setup is short
    static const uint8_t TTL = 60;
    static const size_t ANSWER = 16;

    static uint16_t read16(const uint8_t *p) { return (uint16_t)(p[0] << 8 | p[1]); }

    /** Where the question starting at HEADER ends, 0 if it runs past length or is malformed. */
    static size_t questionEnd(const uint8_t *query, size_t length)
    {
        size_t pos = HEADER;
        for (;;)
        {
            if (pos >= length)
            {
                return 0;
            }
            const uint8_t label = query[pos];
            if (label == 0)
            {
                break;
            }
            // 64 and up are compression pointers, which a question doesn't use
            if (label > 63)
            {
                return 0;
            }
            pos += 1 + label;
        }
        pos += 1 + 4; // the root label, type and class
        return pos <= length ? pos : 0;
    }

    size_t reply(const uint8_t *query, size_t length, const uint8_t ip[4], uint8_t *out, size_t outSize)
    {
        if (length < HEADER)
        {
            return 0;
        }
        const bool isResponse = (query[2] & 0x80) != 0;
        const uint8_t opcode = (query[2] >> 3) & 0x0F;
        if (isResponse || opcode != 0 || read16(query + 4) != 1)
        {
            return 0;
        }

        const size_t end = questionEnd(query, length);
        if (end == 0)
        {
            return 0;
        }
        const bool answer = read16(query + end - 4) == TYPE_A && read16(query + end - 2) == CLASS_IN;
        const size_t size = end + (answer ? ANSWER : 0);
        if (size > outSize)
        {
            return 0;
        }

        // the header and question as asked, without any other records
        memcpy(out, query, end);
        out[2] = 0x84 | (query[2] & 0x01); // response, authoritative, recursion desired as asked
        out[3] = 0x80;                     // recursion available, no error
        out[6] = 0;
        out[7] = answer ? 1 : 0;
        memset(out + 8, 0, 4); // no authority or additional records
        if (answer)
        {
            // the name as a pointer to the question's
            const uint8_t record[ANSWER] = {0xC0, 0x0C, 0, TYPE_A, 0, CLASS_IN, 0, 0, 0, TTL, 0, 4,
                                            ip[0], ip[1], ip[2], ip[3]};
            memcpy(out + end, record, ANSWER);
        }
        return size;
    }
}
