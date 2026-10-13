#ifndef NATIVE

#include <Arduino.h>

#include "display.h"
#include "interface.h"
#include "embedded/blocking_screens.h"

namespace BlockingScreens
{
    void waitForRelease()
    {
        while (Interface::isEncoderPressed())
        {
            Interface::update();
            delay(10);
        }
        Interface::discardClicks();
    }

    void waitForClick()
    {
        waitForRelease();
        while (Interface::getEncoderClick() != ClickType::SINGLE)
        {
            Interface::update();
            delay(10);
        }
        Interface::discardClicks();
    }

    void show(const std::function<void(Canvas &)> &draw)
    {
        Canvas &canvas = Display::canvas();
        canvas.clear();
        draw(canvas);
        canvas.flush();
    }

    uint8_t choose(uint8_t count, const std::function<void(Canvas &, uint8_t)> &draw)
    {
        waitForRelease();
        int index = 0;
        for (;;)
        {
            Interface::update();
            index += Interface::getEncoderTurns();
            index = index < 0 ? 0 : index >= count ? count - 1 : index;
            show([&](Canvas &canvas) { draw(canvas, index); });
            if (Interface::getEncoderClick() == ClickType::SINGLE)
            {
                break;
            }
            delay(10);
        }
        Interface::discardClicks();
        return index;
    }
}

#endif
