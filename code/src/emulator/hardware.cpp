#ifdef EMULATOR

#include <array>
#include <cmath>

#include "emulator/emulator.h"
#include "battery.h"
#include "button.h"
#include "display.h"
#include "interface.h"
#include "loadcell.h"
#include "millis.h"
#include "settings.h"

// A click shorter than the button's debounce time would get lost, e.g. when
// a key is pressed and released between two loops.
#define MIN_PRESS_MS 100

namespace Emulator
{
    SimulatedScale scale;

    static long encoderPosition = 0;
    static bool buttonDown = false;
    static unsigned long pressTime = 0;
    static Button encoderButton;
    static unsigned long buzzerEndTime = 0;

    void turnEncoder(int detents) { encoderPosition += detents; }

    void setButtonPressed(bool pressed)
    {
        if (pressed && !buttonDown)
        {
            pressTime = now();
        }
        buttonDown = pressed;
    }

    bool isButtonPressed() { return buttonDown || now() - pressTime < MIN_PRESS_MS; }

    bool isBuzzing() { return now() < buzzerEndTime; }

    VirtualDisplay &display()
    {
        static VirtualDisplay display;
        return display;
    }
}

namespace Interface
{
    // like the RotaryEncoder library: the direction since the last call
    static long lastDirectionPosition = 0;

    void begin() {}

    void update() { Emulator::encoderButton.update(Emulator::isButtonPressed()); }

    EncoderDirection getEncoderDirection()
    {
        const long position = Emulator::encoderPosition;
        const long last = lastDirectionPosition;
        lastDirectionPosition = position;
        if (position > last)
        {
            return EncoderDirection::CW;
        }
        if (position < last)
        {
            return EncoderDirection::CCW;
        }
        return EncoderDirection::NONE;
    }

    long getEncoderTicks() { return Emulator::encoderPosition; }

    void setEncoderTicks(long ticks) { Emulator::encoderPosition = ticks; }

    void resetEncoderTicks() { Emulator::encoderPosition = 0; }

    ClickType consumeEncoderClick() { return Emulator::encoderButton.consumeClickType(); }

    ClickType getEncoderClick() { return Emulator::encoderButton.getClickType(); }

    bool isEncoderPressed() { return Emulator::encoderButton.isPressed(); }

    void buzzerTone(unsigned int durationMs) { Emulator::buzzerEndTime = now() + durationMs; }
}

namespace LoadCell
{
    void begin() {}

    bool isReady() { return Emulator::scale.isReady(); }

    long read() { return Emulator::scale.read(); }
}

namespace Display
{
    void begin() {}

    Canvas &canvas() { return Emulator::display().canvas(); }

    // The splash screen draws with u8g2's Arduino API, which needs the device.
    void drawOpener() {}
}

namespace Battery
{
    void init() {}

    float getVoltage() { return 3.95f; }

    float getPercentage() { return 75; }

    bool isCharging() { return false; }
}

// Kept in memory only, every start begins with the defaults.
namespace Settings
{
    static std::array<float, FLOAT_SETTING_NUM> values = [] {
        std::array<float, FLOAT_SETTING_NUM> v;
        v.fill(NAN);
        return v;
    }();

    float getFloat(FloatSetting s) { return std::isnan(values[s.id]) ? s.def : values[s.id]; }

    void setFloat(FloatSetting s, float value) { values[s.id] = value; }

    void commit() {}
}

#endif
