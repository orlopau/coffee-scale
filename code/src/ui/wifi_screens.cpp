#include <string.h>

#include "data/localization.h"
#include "ui/widgets.h"
#include "ui/wifi_screens.h"

namespace WifiScreens
{
    // version 3 holds 53 bytes at low error correction, the payload has at most 50
    static const uint8_t QR_VERSION = 3;
    static const uint8_t QR_ECC_LOW = 0;

    /**
     * Draws text with its baseline at y, broken after its first '-' if it is
     * wider than maxWidth, e.g. "CoffeeScale-" and "16777215". Returns the
     * baseline of the next line.
     */
    static int drawBroken(Canvas &canvas, const char *text, int x, int y, int maxWidth, int lineHeight)
    {
        const char *dash = strchr(text, '-');
        if (dash == nullptr || canvas.textWidth(text) <= maxWidth)
        {
            canvas.drawText(x, y, text);
            return y + lineHeight;
        }
        char first[24];
        size_t length = dash - text + 1;
        if (length >= sizeof(first))
        {
            length = sizeof(first) - 1;
        }
        memcpy(first, text, length);
        first[length] = '\0';
        canvas.drawText(x, y, first);
        canvas.drawText(x, y + lineHeight, dash + 1);
        return y + 2 * lineHeight;
    }

    void join(Canvas &canvas, const char *qrPayload, const char *name, const char *password)
    {
        const int size = Widgets::qrCode(canvas, qrPayload, 0, 0, QR_VERSION, QR_ECC_LOW);
        const int x = size + 3;
        const int maxWidth = canvas.width() - x;

        canvas.setFont(Font::Tiny);
        const int lineHeight = canvas.ascent() - canvas.descent() + 2;
        int y = canvas.ascent() + 1;
        canvas.drawText(x, y, WIFI_JOIN_NETWORK);
        y = drawBroken(canvas, name, x, y + lineHeight, maxWidth, lineHeight);
        y += lineHeight / 2;
        canvas.drawText(x, y, WIFI_JOIN_PASSWORD);
        drawBroken(canvas, password, x, y + lineHeight, maxWidth, lineHeight);
    }

    void status(Canvas &canvas, const char *text) { Widgets::textWrapped(canvas, text, 20, 2, canvas.width() - 4); }

    void ask(Canvas &canvas, uint8_t index)
    {
        static const char *const options[] = {WIFI_ASK_YES, WIFI_ASK_LATER};
        Widgets::switcher(canvas, WIFI_ASK_TITLE, index, 2, options);
    }
}
