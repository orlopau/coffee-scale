#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace Png
{
    /**
     * Encodes a black and white image as PNG. pixels holds width * height
     * values row by row, non-zero is white. The image data is stored
     * uncompressed, which keeps the encoder free of dependencies and its
     * output byte-for-byte reproducible.
     */
    std::string encode(int width, int height, const std::vector<uint8_t> &pixels);
}
