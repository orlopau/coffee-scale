#pragma once

#include <clib/u8g2.h>
#include "canvas.h"

/**
 * Canvas backed by a full-buffer u8g2 display.
 *
 * Uses only the u8g2 C API, so the same code runs on the device and in a
 * native build against u8g2's in-memory bitmap device.
 *
 * flush() only sends the buffer over I2C when it differs from the last frame
 * sent (plus a refresh once per second), so redrawing an unchanged screen
 * costs almost no bus time.
 */
class U8g2Canvas : public Canvas
{
public:
    explicit U8g2Canvas(u8g2_t *u8g2) : u8g2(u8g2) {}
    ~U8g2Canvas() override { delete[] lastFrame; }

    int width() override;
    int height() override;
    void clear() override;
    void flush() override;
    /** Forces the next flush() to send, e.g. after the display was cleared externally. */
    void invalidate() { hasLastFrame = false; }

    void setFont(Font font) override;
    int ascent() override;
    int descent() override;
    int textWidth(const char *text) override;
    void drawText(int x, int y, const char *text) override;
    void drawGlyph(int x, int y, uint16_t glyph, uint8_t quarterTurns) override;

    void setColor(uint8_t color) override;
    void drawPixel(int x, int y) override;
    void drawLine(int x0, int y0, int x1, int y1) override;
    void drawHLine(int x, int y, int w) override;
    void drawVLine(int x, int y, int h) override;
    void drawBox(int x, int y, int w, int h) override;
    void drawFrame(int x, int y, int w, int h) override;

    /** Number of frames actually sent to the display. */
    unsigned long sentFrames() const { return sent; }

private:
    u8g2_t *u8g2;
    uint8_t *lastFrame = nullptr;
    bool hasLastFrame = false;
    unsigned long lastSendTime = 0;
    unsigned long sent = 0;
};
