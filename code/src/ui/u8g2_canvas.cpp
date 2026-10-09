#include <string.h>

#include "ui/u8g2_canvas.h"
#include "millis.h"

#define REFRESH_INTERVAL_MS 1000

static const uint8_t *fontFor(Font font)
{
    switch (font)
    {
    case Font::Small:
        return u8g2_font_profont12_tf;
    case Font::SmallMedium:
        return u8g2_font_profont15_tf;
    case Font::Body:
        return u8g2_font_6x10_tf;
    case Font::Medium:
        return u8g2_font_profont17_tf;
    case Font::Large:
        return u8g2_font_logisoso20_tf;
    case Font::Mono13:
        return u8g2_font_7x13_tf;
    case Font::Mono18:
        return u8g2_font_9x18_tf;
    case Font::Mono20:
        return u8g2_font_10x20_tf;
    case Font::Number16:
        return u8g2_font_logisoso16_tf;
    case Font::Number18:
        return u8g2_font_logisoso18_tf;
    case Font::Number22:
        return u8g2_font_logisoso22_tf;
    case Font::Number30:
        return u8g2_font_logisoso30_tf;
    case Font::Battery:
        return u8g2_font_battery19_tn;
    }
    return u8g2_font_6x10_tf;
}

int U8g2Canvas::width() { return u8g2_GetDisplayWidth(u8g2); }
int U8g2Canvas::height() { return u8g2_GetDisplayHeight(u8g2); }

void U8g2Canvas::clear() { u8g2_ClearBuffer(u8g2); }

void U8g2Canvas::flush()
{
    const size_t size = 8 * (size_t)u8g2_GetBufferTileHeight(u8g2) * u8g2_GetBufferTileWidth(u8g2);
    uint8_t *buffer = u8g2_GetBufferPtr(u8g2);

    if (lastFrame == nullptr)
    {
        lastFrame = new uint8_t[size];
    }

    // Unchanged frames are skipped, but still resent once in a while so the
    // display recovers if it ever loses its content (e.g. after a brownout).
    unsigned long time = now();
    bool refreshDue = time - lastSendTime >= REFRESH_INTERVAL_MS;
    if (hasLastFrame && !refreshDue && memcmp(lastFrame, buffer, size) == 0)
    {
        return;
    }

    u8g2_SendBuffer(u8g2);
    memcpy(lastFrame, buffer, size);
    hasLastFrame = true;
    lastSendTime = time;
    sent++;
}

void U8g2Canvas::setFont(Font font) { u8g2_SetFont(u8g2, fontFor(font)); }
int U8g2Canvas::ascent() { return u8g2_GetAscent(u8g2); }
int U8g2Canvas::descent() { return u8g2_GetDescent(u8g2); }
int U8g2Canvas::textWidth(const char *text) { return u8g2_GetUTF8Width(u8g2, text); }
void U8g2Canvas::drawText(int x, int y, const char *text) { u8g2_DrawUTF8(u8g2, x, y, text); }
void U8g2Canvas::drawGlyph(int x, int y, uint16_t glyph, uint8_t quarterTurns)
{
    u8g2_SetFontDirection(u8g2, quarterTurns);
    u8g2_DrawGlyph(u8g2, x, y, glyph);
    u8g2_SetFontDirection(u8g2, 0);
}

void U8g2Canvas::setColor(uint8_t color) { u8g2_SetDrawColor(u8g2, color); }
void U8g2Canvas::drawPixel(int x, int y) { u8g2_DrawPixel(u8g2, x, y); }
void U8g2Canvas::drawLine(int x0, int y0, int x1, int y1) { u8g2_DrawLine(u8g2, x0, y0, x1, y1); }
void U8g2Canvas::drawHLine(int x, int y, int w) { u8g2_DrawHLine(u8g2, x, y, w); }
void U8g2Canvas::drawVLine(int x, int y, int h) { u8g2_DrawVLine(u8g2, x, y, h); }
void U8g2Canvas::drawBox(int x, int y, int w, int h) { u8g2_DrawBox(u8g2, x, y, w, h); }
void U8g2Canvas::drawFrame(int x, int y, int w, int h) { u8g2_DrawFrame(u8g2, x, y, w, h); }
