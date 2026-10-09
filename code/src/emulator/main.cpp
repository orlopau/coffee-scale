#ifdef EMULATOR

// The scale's firmware on a computer: runs the modes like on the device, in
// a window, with the keyboard as encoder and a simulated load cell.
//
//   pio run -e emulator -t exec

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <cstdio>
#include <fstream>
#include <string>

#include "emulator/emulator.h"
#include "battery.h"
#include "data/recipes.h"
#include "display.h"
#include "interface.h"
#include "loadcell.h"
#include "millis.h"
#include "mode_manager.h"
#include "modes/mode_calibrate.h"
#include "modes/mode_espresso.h"
#include "modes/mode_recipe.h"
#include "modes/mode_scale.h"
#include "modes/mode_settings.h"
#include "settings.h"
#include "stopwatch.h"
#include "weight_sensor.h"

// window pixels per display pixel, and the frame around the display
#define PIXEL_SIZE 4
#define BORDER 12
#define FRAME_MS 15
// liquid flowing onto the scale while pouring, in g/s
#define POUR_FLOW 2.0f

static const int SPEEDS[] = {1, 2, 5, 10, 20};
static const int SPEED_COUNT = sizeof(SPEEDS) / sizeof(SPEEDS[0]);

static const char *HELP = "Coffee Scale emulator\n"
                          "  Left/Right       turn the encoder\n"
                          "  Enter, Space     press the encoder (hold for a long press)\n"
                          "  Up/Down          put 1 g on the scale or take it off, with Shift 0.1 g\n"
                          "  PageUp/PageDown  10 g, Home 100 g (e.g. for calibration)\n"
                          "  0, Backspace     clear the scale\n"
                          "  F                pour on/off (2 g/s)\n"
                          "  S                espresso shot (6 s pre-infusion, about 38 g in 33 s)\n"
                          "  N                noise on/off\n"
                          "  - +              slower/faster time, up to 20x (1x while the button is held)\n"
                          "  P                save a screenshot\n"
                          "  Esc, Q           quit\n";

static DefaultWeightSensor weightSensor;
static Stopwatch stopwatch;

static void saveScale(float scale)
{
    printf("New scale: %f\n", scale);
    weightSensor.setScale(scale);
}

/** As the device's setup() does, with the scale calibrated already. */
static void setup()
{
    Interface::begin();
    Battery::init();
    Display::begin();
    weightSensor.begin();

    const float scale = 1 / SimulatedScale::COUNTS_PER_GRAM;
    weightSensor.setScale(scale);
    const float delta = fabs(Settings::getFloat(Settings::floatSettings[Settings::AVERAGING_DELTA]) / scale);
    weightSensor.setAutoAveraging(delta, 64);

    // tare after 32 samples
    for (int i = 0; i < 32;)
    {
        sleep_for(1);
        weightSensor.update();
        if (weightSensor.isNewWeight())
        {
            i++;
        }
    }
    weightSensor.tare();
}

/** One pass of the device's loop(). */
static void loop(ModeManager &modeManager)
{
    Interface::update();
    weightSensor.update();
    modeManager.update();
}

static void saveScreenshot()
{
    static int count = 0;
    std::string path;
    // the next free name, so earlier screenshots are kept
    do
    {
        path = "screenshot-" + std::to_string(++count) + ".png";
    } while (std::ifstream(path).good());
    std::ofstream(path, std::ios::binary) << Emulator::display().png(PIXEL_SIZE);
    printf("Saved %s\n", path.c_str());
}

static void updateTitle(SDL_Window *window, int speed)
{
    char title[128];
    int length = snprintf(title, sizeof(title), "Coffee Scale - %.1f g on the scale", Emulator::scale.getWeight());
    if (Emulator::scale.isShotRunning())
    {
        length += snprintf(title + length, sizeof(title) - length, " - shot, %.1f g/s", Emulator::scale.getFlow());
    }
    else if (Emulator::scale.getFlow() != 0)
    {
        length += snprintf(title + length, sizeof(title) - length, " - pouring %.1f g/s", Emulator::scale.getFlow());
    }
    if (Emulator::scale.hasNoise())
    {
        length += snprintf(title + length, sizeof(title) - length, " - noise");
    }
    if (speed != 1)
    {
        snprintf(title + length, sizeof(title) - length, " - %dx", speed);
    }

    static std::string shown;
    if (shown != title)
    {
        SDL_SetWindowTitle(window, title);
        shown = title;
    }
}

static void draw(SDL_Renderer *renderer, SDL_Texture *texture)
{
    VirtualDisplay &display = Emulator::display();
    uint32_t *pixels;
    int pitch;
    SDL_LockTexture(texture, nullptr, (void **)&pixels, &pitch);
    for (int y = 0; y < display.height(); y++)
    {
        for (int x = 0; x < display.width(); x++)
        {
            pixels[y * pitch / 4 + x] = display.pixel(x, y) ? 0xFFE8F0FF : 0xFF000000;
        }
    }
    SDL_UnlockTexture(texture);

    // the frame lights up while the buzzer sounds
    if (Emulator::isBuzzing())
    {
        SDL_SetRenderDrawColor(renderer, 0xF0, 0xA0, 0x20, 0xFF);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 0x30, 0x30, 0x30, 0xFF);
    }
    SDL_RenderClear(renderer);
    const SDL_Rect screen = {BORDER, BORDER, display.width() * PIXEL_SIZE, display.height() * PIXEL_SIZE};
    SDL_RenderCopy(renderer, texture, nullptr, &screen);
    SDL_RenderPresent(renderer);
}

/** Handles a key, returns false to quit. */
static bool handleKey(const SDL_KeyboardEvent &key, int &speedIndex)
{
    const bool down = key.type == SDL_KEYDOWN;
    const bool shift = key.keysym.mod & KMOD_SHIFT;
    SimulatedScale &scale = Emulator::scale;

    switch (key.keysym.sym)
    {
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
    case SDLK_SPACE:
        Emulator::setButtonPressed(down);
        return true;
    case SDLK_ESCAPE:
    case SDLK_q:
        return !down;
    }

    if (!down)
    {
        return true;
    }

    // keys that may repeat while held
    switch (key.keysym.sym)
    {
    case SDLK_RIGHT:
        Emulator::turnEncoder(1);
        break;
    case SDLK_LEFT:
        Emulator::turnEncoder(-1);
        break;
    case SDLK_UP:
        scale.addWeight(shift ? 0.1f : 1);
        break;
    case SDLK_DOWN:
        scale.addWeight(shift ? -0.1f : -1);
        break;
    case SDLK_PAGEUP:
        scale.addWeight(10);
        break;
    case SDLK_PAGEDOWN:
        scale.addWeight(-10);
        break;
    case SDLK_HOME:
        scale.addWeight(100);
        break;
    }

    if (key.repeat)
    {
        return true;
    }

    switch (key.keysym.sym)
    {
    case SDLK_0:
    case SDLK_KP_0:
    case SDLK_BACKSPACE:
        scale.setWeight(0);
        break;
    case SDLK_f:
        scale.setFlow(scale.getFlow() == 0 || scale.isShotRunning() ? POUR_FLOW : 0);
        break;
    case SDLK_s:
        scale.startShot();
        break;
    case SDLK_n:
        scale.setNoise(!scale.hasNoise());
        break;
    case SDLK_MINUS:
    case SDLK_KP_MINUS:
        speedIndex = speedIndex > 0 ? speedIndex - 1 : 0;
        break;
    case SDLK_PLUS:
    case SDLK_EQUALS: // + without Shift on US keyboards
    case SDLK_KP_PLUS:
        speedIndex = speedIndex < SPEED_COUNT - 1 ? speedIndex + 1 : SPEED_COUNT - 1;
        break;
    case SDLK_p:
        saveScreenshot();
        break;
    }
    return true;
}

int main(int argc, char **argv)
{
    setup();

    ModeScale modeDefault(weightSensor, stopwatch);
    ModeEspresso modeEspresso(weightSensor, stopwatch);
    ModeCalibration modeCalibration(stopwatch, saveScale);
    ModeSettings modeSettings;
    ModeRecipes modeRecipes(weightSensor, RECIPES, RECIPE_COUNT);
    Mode *modes[] = {&modeDefault, &modeRecipes, &modeEspresso, &modeCalibration, &modeSettings};
    ModeManager modeManager(modes, 5, Display::canvas());
    modeManager.begin();

    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    VirtualDisplay &display = Emulator::display();
    SDL_Window *window = SDL_CreateWindow("Coffee Scale", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          display.width() * PIXEL_SIZE + 2 * BORDER,
                                          display.height() * PIXEL_SIZE + 2 * BORDER, 0);
    SDL_Renderer *renderer = window ? SDL_CreateRenderer(window, -1, 0) : nullptr;
    SDL_Texture *texture = renderer ? SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                                        display.width(), display.height())
                                    : nullptr;
    if (texture == nullptr)
    {
        fprintf(stderr, "Creating the window failed: %s\n", SDL_GetError());
        return 1;
    }
    printf("%s", HELP);
    fflush(stdout);

    int speedIndex = 0;
    Uint64 lastRealTime = SDL_GetTicks64();
    Uint64 pendingMs = 0;
    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (running && SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                running = handleKey(event.key, speedIndex);
            }
        }

        // The virtual clock catches up with real time, times the speed, in
        // steps of 1 ms. While the button is held, time runs at normal speed,
        // so a long press takes as long as on the device.
        const Uint64 realTime = SDL_GetTicks64();
        const Uint64 elapsed = SDL_min(realTime - lastRealTime, 100); // e.g. after the window was dragged
        lastRealTime = realTime;
        const int speed = Emulator::isButtonPressed() ? 1 : SPEEDS[speedIndex];
        pendingMs += elapsed * speed;
        for (; pendingMs > 0; pendingMs--)
        {
            sleep_for(1);
            Emulator::scale.update();
            loop(modeManager);
        }

        updateTitle(window, SPEEDS[speedIndex]);
        draw(renderer, texture);
        SDL_Delay(FRAME_MS);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

#endif
