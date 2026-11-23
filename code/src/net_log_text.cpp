#include "net_log_text.h"

#include <inttypes.h>
#include <stdio.h>

namespace NetLogText
{
    size_t stripColors(char *text, size_t length)
    {
        size_t out = 0;
        size_t in = 0;
        while (in < length)
        {
            if (text[in] != '\033')
            {
                text[out++] = text[in++];
                continue;
            }
            in++; // ESC
            if (in < length && text[in] == '[')
            {
                in++;
                // Parameter bytes up to and including the final byte.
                while (in < length)
                {
                    char c = text[in++];
                    if (c >= 0x40 && c <= 0x7E)
                    {
                        break;
                    }
                }
            }
        }
        return out;
    }

    size_t droppedLine(uint32_t count, char *out, size_t size)
    {
        if (size == 0)
        {
            return 0;
        }
        int written = snprintf(out, size, "net_log: %" PRIu32 " lines dropped\n", count);
        if (written < 0)
        {
            out[0] = '\0';
            return 0;
        }
        return (size_t)written < size ? (size_t)written : size - 1;
    }
}
