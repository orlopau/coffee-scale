#include "ui/updater_screens.h"
#include "ui/widgets.h"

namespace UpdaterScreens
{
    void message(Canvas &canvas, const char *text)
    {
        canvas.setFont(Font::Mono13);
        Widgets::textHCentered(canvas, text, 38);
    }

    void text(Canvas &canvas, const char *text) { Widgets::textLines(canvas, text); }

    void switcher(Canvas &canvas, const char *title, uint8_t index, uint8_t count, const char *const options[])
    {
        Widgets::switcher(canvas, title, index, count, options);
    }
}
