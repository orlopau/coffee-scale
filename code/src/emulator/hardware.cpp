#ifdef EMULATOR

#include <array>
#include <cmath>
#include <deque>

#include "emulator/emulator.h"
#include "battery.h"
#include "button.h"
#include "display.h"
#include "input/click_inbox.h"
#include "interface.h"
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

    /** The emulated button's clicks, like the device's click queue. */
    class ButtonClicks : public ClickSource
    {
    public:
        std::deque<ClickEvent> events;

        bool next(ClickEvent &event) override
        {
            if (events.empty())
            {
                return false;
            }
            event = events.front();
            events.pop_front();
            return true;
        }
    };

    static ButtonClicks clicks;
    static ClickInbox clickInbox(clicks);
    static unsigned long buzzStart = 0;
    static unsigned int buzzCount = 0;
    static unsigned int buzzOnMs = 0;
    static unsigned int buzzOffMs = 0;

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

    bool isBuzzing()
    {
        if (buzzCount == 0 || buzzOnMs == 0)
        {
            return false;
        }
        const unsigned long elapsed = now() - buzzStart;
        const unsigned long period = buzzOnMs + buzzOffMs;
        return elapsed < period * (buzzCount - 1) + buzzOnMs && elapsed % period < buzzOnMs;
    }

    VirtualDisplay &display()
    {
        static VirtualDisplay display;
        return display;
    }
}

namespace Interface
{
    static long lastPosition = 0;
    static int turns = 0;
    static long ticks = 0;

    void begin() {}

    void update()
    {
        // the device's button task does this every 5 ms, here once per loop
        Emulator::encoderButton.update(Emulator::isButtonPressed());
        ClickType type = Emulator::encoderButton.getClickType();
        if (type != ClickType::NONE)
        {
            Emulator::clicks.events.push_back({static_cast<uint32_t>(now()), type});
        }

        turns = static_cast<int>(Emulator::encoderPosition - lastPosition);
        lastPosition = Emulator::encoderPosition;
        ticks += turns;

        Emulator::clickInbox.update(now());
    }

    int getEncoderTurns() { return turns; }

    long getEncoderTicks() { return ticks; }

    void setEncoderTicks(long value) { ticks = value; }

    void resetEncoderTicks() { ticks = 0; }

    ClickType consumeEncoderClick() { return Emulator::clickInbox.consume(); }

    ClickType getEncoderClick() { return Emulator::clickInbox.get(); }

    void discardClicks() { Emulator::clickInbox.discard(); }

    bool isEncoderPressed() { return Emulator::encoderButton.isPressed(); }

    void buzzerPattern(unsigned int count, unsigned int onMs, unsigned int offMs)
    {
        Emulator::buzzStart = now();
        Emulator::buzzCount = count;
        Emulator::buzzOnMs = onMs;
        Emulator::buzzOffMs = offMs;
    }

    void buzzerTone(unsigned int durationMs) { buzzerPattern(1, durationMs, 0); }
}

namespace Emulator
{
    class SimulatedSamples : public SampleSource
    {
    public:
        bool next(Sample &sample) override
        {
            if (!scale.isReady())
            {
                return false;
            }
            sample = {static_cast<uint32_t>(now()), static_cast<int32_t>(scale.read())};
            return true;
        }
    };

    SampleSource &samples()
    {
        static SimulatedSamples samples;
        return samples;
    }
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

    // -1: nothing stored
    static std::array<int, CHOICE_SETTING_NUM> choices = [] {
        std::array<int, CHOICE_SETTING_NUM> c;
        c.fill(-1);
        return c;
    }();

    uint8_t getChoice(ChoiceSetting s) { return validChoice(s, choices[s.id]); }

    void setChoice(ChoiceSetting s, uint8_t index) { choices[s.id] = index; }
}

#endif
