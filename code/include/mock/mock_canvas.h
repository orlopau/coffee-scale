#pragma once

#include <string>
#include <vector>
#include "canvas.h"

/**
 * Canvas for native tests. Records the text drawn in the current frame and
 * counts frames, using fixed font metrics (6px per character).
 */
class RecordingCanvas : public Canvas
{
public:
    struct Text
    {
        int x, y;
        std::string text;
    };

    std::vector<Text> texts;
    struct Box
    {
        int x, y, w, h;
    };
    int boxes = 0;
    std::vector<Box> boxList;
    int clears = 0;
    int flushes = 0;
    Font font = Font::Body;

    int width() override { return 128; }
    int height() override { return 64; }
    void clear() override
    {
        texts.clear();
        boxes = 0;
        boxList.clear();
        clears++;
    }
    void flush() override { flushes++; }

    void setFont(Font f) override { font = f; }
    int ascent() override { return 10; }
    int descent() override { return -2; }
    int textWidth(const char *text) override { return 6 * (int)std::string(text).size(); }
    void drawText(int x, int y, const char *text) override { texts.push_back({x, y, text}); }
    void drawGlyph(int x, int y, uint16_t glyph, uint8_t quarterTurns) override {}

    void setColor(uint8_t color) override {}
    void drawPixel(int x, int y) override {}
    void drawLine(int x0, int y0, int x1, int y1) override {}
    void drawHLine(int x, int y, int w) override {}
    void drawVLine(int x, int y, int h) override {}
    void drawBox(int x, int y, int w, int h) override {
        boxes++;
        boxList.push_back({x, y, w, h});
    }
    void drawFrame(int x, int y, int w, int h) override {}

    /** True if some text drawn in the current frame equals text exactly. */
    bool hasText(const char *text) const
    {
        for (const Text &t : texts)
        {
            if (t.text == text)
            {
                return true;
            }
        }
        return false;
    }

    /** True if a box of exactly this size was drawn at x, y in the current frame. */
    bool hasBox(int x, int y, int w, int h) const
    {
        for (const Box &b : boxList)
        {
            if (b.x == x && b.y == y && b.w == w && b.h == h)
            {
                return true;
            }
        }
        return false;
    }

    void reset()
    {
        texts.clear();
        boxes = clears = flushes = 0;
        boxList.clear();
        font = Font::Body;
    }
};
