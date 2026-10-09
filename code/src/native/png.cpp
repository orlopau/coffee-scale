#ifdef NATIVE

#include "native/png.h"

namespace Png
{
    static uint32_t crc32(const std::string &data, size_t start)
    {
        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = start; i < data.size(); i++)
        {
            crc ^= (uint8_t)data[i];
            for (int k = 0; k < 8; k++)
            {
                crc = (crc >> 1) ^ (0xEDB88320 & (0 - (crc & 1)));
            }
        }
        return ~crc;
    }

    static uint32_t adler32(const std::string &data)
    {
        uint32_t a = 1, b = 0;
        for (char c : data)
        {
            a = (a + (uint8_t)c) % 65521;
            b = (b + a) % 65521;
        }
        return (b << 16) | a;
    }

    static void put32(std::string &out, uint32_t value)
    {
        for (int shift = 24; shift >= 0; shift -= 8)
        {
            out += (char)(value >> shift);
        }
    }

    static void chunk(std::string &out, const char *type, const std::string &data)
    {
        put32(out, data.size());
        const size_t start = out.size();
        out += type;
        out += data;
        put32(out, crc32(out, start));
    }

    std::string encode(int width, int height, const std::vector<uint8_t> &pixels)
    {
        // scanlines: filter type 0, then 1 bit per pixel, most significant bit first
        const int rowBytes = (width + 7) / 8;
        std::string raw;
        for (int y = 0; y < height; y++)
        {
            raw += '\0';
            for (int byte = 0; byte < rowBytes; byte++)
            {
                uint8_t bits = 0;
                for (int bit = 0; bit < 8; bit++)
                {
                    const int x = byte * 8 + bit;
                    if (x < width && pixels[y * width + x])
                    {
                        bits |= 0x80 >> bit;
                    }
                }
                raw += (char)bits;
            }
        }

        // zlib stream of uncompressed deflate blocks
        std::string zlib = "\x78\x01";
        size_t pos = 0;
        do
        {
            const size_t len = std::min<size_t>(raw.size() - pos, 65535);
            const bool last = pos + len == raw.size();
            zlib += (char)(last ? 1 : 0);
            zlib += (char)(len & 0xFF);
            zlib += (char)(len >> 8);
            zlib += (char)(~len & 0xFF);
            zlib += (char)((~len >> 8) & 0xFF);
            zlib.append(raw, pos, len);
            pos += len;
        } while (pos < raw.size());
        put32(zlib, adler32(raw));

        std::string header;
        put32(header, width);
        put32(header, height);
        header += '\x01'; // bit depth
        header += '\x00'; // grayscale
        header += std::string(3, '\0'); // compression, filter, interlace

        std::string png = "\x89PNG\r\n\x1a\n";
        chunk(png, "IHDR", header);
        chunk(png, "IDAT", zlib);
        chunk(png, "IEND", "");
        return png;
    }
}

#endif
