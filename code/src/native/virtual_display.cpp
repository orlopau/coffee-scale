#ifdef NATIVE

#include "native/virtual_display.h"
#include "native/png.h"

// The display is connected through nothing, every byte sent is dropped.
static uint8_t noIo(u8x8_t *, uint8_t, uint8_t, void *) { return 1; }

VirtualDisplay::VirtualDisplay() : u8g2Canvas(&u8g2)
{
    // as U8G2_SH1107_64X128_F_HW_I2C(U8G2_R1, ...) does on the device
    u8g2_Setup_sh1107_i2c_64x128_f(&u8g2, U8G2_R1, noIo, noIo);
    u8g2_ClearBuffer(&u8g2);
}

bool VirtualDisplay::pixel(int x, int y)
{
    // The buffer is in the controller's orientation, 64 wide and 128 high,
    // each byte holds 8 pixels of a column. U8G2_R1 turns the screen
    // clockwise, so screen pixel x, y is buffer pixel 63 - y, x.
    const int bufferWidth = 8 * u8g2_GetBufferTileWidth(&u8g2);
    const int bx = bufferWidth - 1 - y;
    const int by = x;
    const uint8_t *buffer = u8g2_GetBufferPtr(&u8g2);
    return (buffer[(by / 8) * bufferWidth + bx] >> (by % 8)) & 1;
}

std::string VirtualDisplay::png(int scale)
{
    const int w = width() * scale, h = height() * scale;
    std::vector<uint8_t> pixels(w * h);
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            pixels[y * w + x] = pixel(x / scale, y / scale);
        }
    }
    return Png::encode(w, h, pixels);
}

#endif
