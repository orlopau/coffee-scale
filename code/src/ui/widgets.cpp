#include <string.h>
#include <qrcode.h>

#include "ui/widgets.h"
#include "millis.h"

namespace Widgets
{
    void textHCentered(Canvas &canvas, const char *text, int y)
    {
        canvas.drawText((int)(canvas.width() / 2.0 - canvas.textWidth(text) / 2.0), y, text);
    }

    void textCentered(Canvas &canvas, const char *text)
    {
        textHCentered(canvas, text, (int)(canvas.height() / 2.0 + canvas.ascent() / 2.0));
    }

    int titleLine(Canvas &canvas, const char *title)
    {
        static const int PADDING = 4;

        canvas.setFont(Font::Small);
        int y = canvas.ascent() - canvas.descent();
        textHCentered(canvas, title, y);
        y += PADDING;
        canvas.drawHLine(0, y, canvas.width());
        return y;
    }

    int segmentBar(Canvas &canvas, uint8_t index, uint8_t count)
    {
        static const int HEIGHT = 4;

        if (count == 0)
        {
            return HEIGHT;
        }

        int width = canvas.width();
        int segmentWidth = width / count;
        canvas.drawFrame(0, 0, width, HEIGHT);
        for (int i = 1; i < count; i++)
        {
            canvas.drawVLine(i * segmentWidth, 0, HEIGHT);
        }
        canvas.drawBox(index * segmentWidth, 0, segmentWidth, HEIGHT);
        return HEIGHT;
    }

    void progressBar(Canvas &canvas, int x, int y, int w, int h, float fraction)
    {
        if (!(fraction > 0)) // also catches NaN
        {
            fraction = 0;
        }
        else if (fraction > 1)
        {
            fraction = 1;
        }

        canvas.drawFrame(x, y, w, h);
        int filled = (int)(fraction * w);
        if (filled > 0)
        {
            canvas.drawBox(x, y, filled, h);
        }
    }

    int textWrapped(Canvas &canvas, const char *text, int yTop, int xLeft, int maxWidth)
    {
        canvas.setFont(Font::Body);
        int lineHeight = canvas.ascent() - canvas.descent();
        int spaceWidth = canvas.textWidth(" ");

        int line = yTop + lineHeight;
        int x = xLeft;

        // words longer than this cannot fit a line on the display anyway
        char word[64];
        const char *p = text;
        while (*p != '\0')
        {
            while (*p == ' ')
            {
                p++;
            }
            if (*p == '\0')
            {
                break;
            }

            size_t len = strcspn(p, " ");
            size_t copied = len < sizeof(word) - 1 ? len : sizeof(word) - 1;
            memcpy(word, p, copied);
            word[copied] = '\0';
            p += len;

            int wordWidth = canvas.textWidth(word);
            if (x + wordWidth > maxWidth)
            {
                x = xLeft;
                line += lineHeight;
            }
            canvas.drawText(x, line, word);
            x += wordWidth + spaceWidth;
        }

        return line;
    }

    void textLines(Canvas &canvas, const char *text)
    {
        static const int LINE_HEIGHT = 10;

        canvas.setFont(Font::Body);
        char line[64];
        int y = LINE_HEIGHT;
        const char *p = text;
        while (*p != '\0')
        {
            size_t len = strcspn(p, "\n");
            if (len > 0)
            {
                size_t copied = len < sizeof(line) - 1 ? len : sizeof(line) - 1;
                memcpy(line, p, copied);
                line[copied] = '\0';
                canvas.drawText(0, y, line);
                y += LINE_HEIGHT;
            }
            p += len;
            if (*p == '\n')
            {
                p++;
            }
        }
    }

    void switcher(Canvas &canvas, const char *title, uint8_t index, uint8_t count, const char *const options[])
    {
        int y = titleLine(canvas, title) + 2;

        canvas.setFont(Font::Small);
        int ascent = canvas.ascent();
        int optionHeight = ascent + 2;
        int visibleCount = (canvas.height() - y) / optionHeight;
        if (visibleCount > count)
        {
            visibleCount = count;
        }

        int first = index - visibleCount / 2;
        if (first < 0)
        {
            first = 0;
        }
        else if (first + visibleCount > count)
        {
            // start at the last possible option instead of showing empty space at the end
            first = count - visibleCount;
        }

        // y is the top of the current option
        for (int i = 0; i < visibleCount; i++)
        {
            int option = first + i;
            if (option == index)
            {
                canvas.drawBox(0, y, canvas.width(), optionHeight);
                canvas.setColor(0);
            }
            canvas.drawText(2, y + ascent + 1, options[option]);
            canvas.setColor(1);
            y += optionHeight;
        }
    }

    int qrCode(Canvas &canvas, const char *text, int x, int y)
    {
        static const uint8_t VERSION = 2;
        static const int BORDER = 2;
        static QRCode qrcode;
        static uint8_t *bytes = new uint8_t[qrcode_getBufferSize(VERSION)];
        // encoding is the expensive part, and screens redraw every frame
        static char encoded[32] = "";

        if (strncmp(encoded, text, sizeof(encoded)) != 0)
        {
            qrcode_initText(&qrcode, bytes, VERSION, ECC_QUARTILE, text);
            strncpy(encoded, text, sizeof(encoded) - 1);
        }

        // modules are drawn off on a filled box, with a border around them
        int size = 2 * qrcode.size + 2 * BORDER;
        canvas.drawBox(x, y, size, size);
        canvas.setColor(0);
        for (uint8_t my = 0; my < qrcode.size; my++)
        {
            for (uint8_t mx = 0; mx < qrcode.size; mx++)
            {
                if (qrcode_getModule(&qrcode, mx, my))
                {
                    canvas.drawBox(2 * mx + x + BORDER, 2 * my + y + BORDER, 2, 2);
                }
            }
        }
        canvas.setColor(1);
        return size;
    }

    int centerBaseline(Canvas &canvas, int y)
    {
        // same as u8g2's setFontPosCenter()
        return y + (canvas.ascent() - canvas.descent()) / 2 + canvas.descent();
    }

    bool blinkVisible() { return now() % 1000 > 200; }
}
