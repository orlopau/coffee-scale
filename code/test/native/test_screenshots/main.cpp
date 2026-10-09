// Screenshot tests: renders every screen through u8g2, set up like the scale's
// display, and compares it with the reference images in screens/.
//
// After changing a screen on purpose, accept the new images with
//   UPDATE_SCREENSHOTS=1 pio test -e native -f native/test_screenshots
// and review them in the diff. Images that do not match are written to failed/.

#include <unity.h>

#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "native/virtual_display.h"
#include "mocks.h"
#include "mock/mock_battery.h"
#include "mock/mock_interface.h"
#include "mock/mock_loadcell.h"
#include "millis.h"
#include "mode_manager.h"
#include "data/localization.h"
#include "data/recipes.h"
#include "modes/mode_scale.h"
#include "modes/mode_espresso.h"
#include "modes/mode_calibrate.h"
#include "modes/mode_settings.h"
#include "modes/mode_recipe.h"
#include "modes/steps/step_switcher.h"
#include "modes/steps/step_summary.h"
#include "modes/steps/step_config_ratio.h"
#include "modes/steps/step_config_weight.h"
#include "modes/steps/step_prepare.h"
#include "modes/steps/step_brewing.h"
#include "modes/steps/step_done.h"
#include "ui/updater_screens.h"

namespace fs = std::filesystem;

// Each display pixel becomes 2x2 pixels in the images, so they are readable in diffs.
static const int SCALE = 2;

static const fs::path HERE = fs::path(__FILE__).parent_path();
static const fs::path SCREENS_DIR = HERE / "screens";
static const fs::path FAILED_DIR = HERE / "failed";

static VirtualDisplay display;
static MockWeightSensor *weightSensor;
static Stopwatch *stopwatch;

static bool updating = false;
static std::set<std::string> checkedScreens;
// screens of the current test that differ from their reference
static std::vector<std::string> mismatches;

static std::string readFile(const fs::path &path)
{
    std::ifstream in(path, std::ios::binary);
    std::stringstream content;
    content << in.rdbuf();
    return content.str();
}

static void writeFile(const fs::path &path, const std::string &content)
{
    fs::create_directories(path.parent_path());
    std::ofstream(path, std::ios::binary) << content;
}

/** Compares the current frame with screens/<name>.png, or replaces it when updating. */
static void check(const std::string &name)
{
    TEST_ASSERT_TRUE_MESSAGE(checkedScreens.insert(name).second, ("screen name used twice: " + name).c_str());

    const std::string png = display.png(SCALE);
    const fs::path reference = SCREENS_DIR / (name + ".png");
    if (updating)
    {
        writeFile(reference, png);
    }
    else if (!fs::exists(reference))
    {
        writeFile(FAILED_DIR / (name + ".png"), png);
        mismatches.push_back(name + " (no reference image)");
    }
    else if (readFile(reference) != png)
    {
        writeFile(FAILED_DIR / (name + ".png"), png);
        mismatches.push_back(name);
    }
}

/** Renders the current state of a mode or recipe step and checks it. */
template <typename Screen>
static void render(const std::string &name, Screen &screen)
{
    display.canvas().clear();
    screen.render(display.canvas());
    check(name);
}

/** Updates a mode or recipe step without input, then renders and checks it. */
template <typename Screen>
static void show(const std::string &name, Screen &screen)
{
    Interface::reset();
    screen.update();
    render(name, screen);
}

template <typename Screen>
static void turn(Screen &screen, int steps)
{
    for (int i = 0; i < abs(steps); i++)
    {
        Interface::reset();
        Interface::encoderDirection = steps > 0 ? Interface::EncoderDirection::CW : Interface::EncoderDirection::CCW;
        screen.update();
    }
    Interface::reset();
}

template <typename Screen>
static void click(Screen &screen, ClickType type = ClickType::SINGLE)
{
    Interface::reset();
    Interface::encoderClick = type;
    screen.update();
    Interface::reset();
}

// Values that blink are visible during the second half of each second.
static void blinkOn() { set_now(now() / 1000 * 1000 + 1500); }
static void blinkOff() { set_now(now() / 1000 * 1000 + 1100); }

static void saveScale(float) {}

void setUp(void)
{
    set_now(1000000);
    Interface::reset();
    Battery::reset();
    LoadCell::value = 0;
    LoadCell::ready = true;
    weightSensor = new MockWeightSensor();
    stopwatch = new Stopwatch();
    mismatches.clear();
}

void tearDown(void)
{
    delete stopwatch;
    delete weightSensor;

    std::string message = "screens differ, see " + FAILED_DIR.string() + ":";
    for (const std::string &name : mismatches)
    {
        message += " " + name;
    }
    TEST_ASSERT_TRUE_MESSAGE(mismatches.empty(), message.c_str());
}

void test_scale(void)
{
    ModeScale scale(*weightSensor, *stopwatch);
    scale.enter();
    show("scale_zero", scale);

    weightSensor->weight = 12.34f;
    stopwatch->start();
    sleep_for(65432);
    show("scale_running", scale);

    weightSensor->weight = -1.5f;
    show("scale_negative", scale);

    weightSensor->weight = 1234.5f;
    sleep_for(3599999 - 65432);
    show("scale_big", scale);
}

void test_espresso(void)
{
    ModeEspresso espresso(*weightSensor, *stopwatch);
    espresso.enter();
    show("espresso_idle", espresso);

    click(espresso);
    for (int i = 1; i <= 8; i++)
    {
        sleep_for(500);
        weightSensor->weight = 2.5f * i;
        weightSensor->newWeight = true;
        espresso.update();
    }
    weightSensor->newWeight = false;
    show("espresso_brewing", espresso);
}

void test_recipe_switcher(void)
{
    RecipeStepState state;
    RecipeSwitcherStep step(state, RECIPES, RECIPE_COUNT);
    step.enter();
    show("recipe_switcher_first", step);
    turn(step, 2);
    show("recipe_switcher_middle", step);
    turn(step, 6);
    show("recipe_switcher_last", step);
}

void test_recipe_summary(void)
{
    RecipeStepState state;
    RecipeSummaryStep step(state);
    for (int r = 0; r < RECIPE_COUNT; r++)
    {
        state.originalRecipe = &RECIPES[r];
        state.configRecipe = RECIPES[r];
        step.enter();
        show("recipe_summary_" + std::to_string(r), step);
    }
}

void test_recipe_config_ratio(void)
{
    RecipeStepState state = {&RECIPES[0], RECIPES[0]};
    RecipeConfigRatioStep step(state);
    step.enter();
    blinkOn();
    show("recipe_ratio_default_on", step);
    blinkOff();
    show("recipe_ratio_default_off", step);

    Interface::reset();
    Interface::encoderTicks = 23;
    step.update();
    blinkOn();
    render("recipe_ratio_adjusted", step);
}

void test_recipe_config_weight(void)
{
    RecipeStepState state = {&RECIPES[0], RECIPES[0]};
    RecipeConfigWeightStep step(state);
    step.enter();
    blinkOn();
    show("recipe_weight_default_on", step);
    blinkOff();
    show("recipe_weight_default_off", step);

    for (int ticks : {5, -3})
    {
        step.enter();
        Interface::reset();
        Interface::encoderTicks = ticks;
        step.update();
        blinkOn();
        render("recipe_weight_adjusted_" + std::string(ticks > 0 ? "up" : "down"), step);
    }
}

void test_recipe_prepare(void)
{
    RecipeStepState state = {&RECIPES[0], RECIPES[0]};
    RecipePrepare step(state, *weightSensor);
    step.enter();
    show("recipe_prepare_empty", step);
    weightSensor->weight = 5.678f;
    show("recipe_prepare_some", step);
    weightSensor->weight = 12.0f;
    show("recipe_prepare_enough", step);
    weightSensor->weight = -0.2f;
    show("recipe_prepare_negative", step);
}

void test_recipe_brewing(void)
{
    // the countdown while pouring (V60 Scott Rao)
    {
        RecipeStepState state = {&RECIPES[3], RECIPES[3]};
        RecipeBrewing step(state, *weightSensor);
        step.enter();
        click(step);
        sleep_for(5000);
        weightSensor->weight = 20.5f;
        show("recipe_brewing_pouring", step);
    }

    // the countdown during a pause (Aeropress)
    {
        RecipeStepState state = {&RECIPES[0], RECIPES[0]};
        RecipeBrewing step(state, *weightSensor);
        step.enter();
        click(step);
        sleep_for(30000);
        weightSensor->weight = 200.4f;
        show("recipe_brewing_pause", step);
    }

    // every pour of every recipe as it comes up, for the notes
    for (int r = 0; r < RECIPE_COUNT; r++)
    {
        RecipeStepState state = {&RECIPES[r], RECIPES[r]};
        RecipeBrewing step(state, *weightSensor);
        step.enter();
        weightSensor->weight = 0;
        for (int p = 0; p < RECIPES[r].poursCount; p++)
        {
            if (p > 0)
            {
                click(step); // on to the next pour
            }
            weightSensor->weight += 17.3f;
            show("recipe_brewing_" + std::to_string(r) + "_pour_" + std::to_string(p), step);
            if (!RECIPES[r].pours[p].autoStart)
            {
                click(step); // start it
            }
        }
    }
}

void test_recipe_done(void)
{
    RecipeStepState state = {&RECIPES[0], RECIPES[0]};
    RecipeDone step(state);
    show("recipe_done", step);
}

void test_calibration(void)
{
    LoadCell::value = 1000;
    ModeCalibration calibration(*stopwatch, saveScale);
    show("calibration_begin", calibration);
    click(calibration);
    show("calibration_add_weight", calibration);
    click(calibration);
    LoadCell::ready = false;
    show("calibration_calibrating", calibration);
    LoadCell::ready = true;
    LoadCell::value = 51000;
    for (int i = 0; i < CALIBRATION_SAMPLE_SIZE; i++)
    {
        Interface::reset();
        calibration.update();
    }
    show("calibration_end", calibration);
}

void test_settings(void)
{
    ModeSettings settings;
    settings.enter();
    show("settings_first", settings);
    turn(settings, 3);
    show("settings_middle", settings);
    turn(settings, 9);
    show("settings_last", settings);

    click(settings);
    for (int ticks : {0, 7, -3})
    {
        Interface::reset();
        Interface::encoderTicks = ticks;
        settings.update();
        render("settings_value_" + std::string(ticks == 0 ? "unchanged" : ticks > 0 ? "up" : "down"), settings);
    }
}

void test_mode_switcher(void)
{
    ModeScale scale(*weightSensor, *stopwatch);
    ModeEspresso espresso(*weightSensor, *stopwatch);
    ModeCalibration calibration(*stopwatch, saveScale);
    ModeSettings settings;
    ModeRecipes recipes(*weightSensor, RECIPES, RECIPE_COUNT);
    Mode *modes[] = {&scale, &recipes, &espresso, &calibration, &settings};
    ModeManager manager(modes, 5, display.canvas());
    manager.begin();
    sleep_for(100);
    click(manager, ClickType::LONG);

    // one battery state per mode
    const struct
    {
        float voltage, percentage;
        bool charging;
    } batteries[] = {{4.05f, 85, false}, {3.7f, 40, true}, {3.31f, 3, false}, {3.9f, 100, false}, {3.5f, 10, false}};
    for (int i = 0; i < 5; i++)
    {
        Battery::voltage = batteries[i].voltage;
        Battery::percentage = batteries[i].percentage;
        Battery::charging = batteries[i].charging;
        sleep_for(BATTERY_UPDATE_INTERVAL + 500);
        Interface::reset();
        manager.update(); // renders, the last frame is long enough ago
        check("mode_switcher_" + std::to_string(i));
        turn(manager, 1);
    }
}

void test_updater(void)
{
    Canvas &canvas = display.canvas();

    const char *languages[] = {"Deutsch", "English"};
    for (int i = 0; i < 2; i++)
    {
        canvas.clear();
        UpdaterScreens::switcher(canvas, "Choose Language", i, 2, languages);
        check("updater_language_" + std::to_string(i));
    }

    const struct
    {
        const char *name, *text;
    } messages[] = {
        {"updating", UPDATER_UPDATING},       {"wifi_connected", UPDATER_WIFI_CONNECTED},
        {"dev_searching", UPDATER_DEV_SEARCHING}, {"dev_not_found", UPDATER_DEV_NOT_FOUND},
        {"failed", UPDATER_FAILED},           {"no_update", UPDATER_NO_UPDATE},
        {"success", UPDATER_SUCCESS},
    };
    for (const auto &message : messages)
    {
        canvas.clear();
        UpdaterScreens::message(canvas, message.text);
        check("updater_" + std::string(message.name));
    }

    char text[128];
    snprintf(text, sizeof(text), UPDATER_PROGRESS, 42.17f);
    canvas.clear();
    UpdaterScreens::message(canvas, text);
    check("updater_progress");

    snprintf(text, sizeof(text), UPDATER_WIFI_CONNECT_MANUAL, "CoffeeScale-1234567");
    canvas.clear();
    UpdaterScreens::text(canvas, text);
    check("updater_wifi_setup");
}

/** Every reference image belongs to a screen, so renamed or removed screens leave none behind. */
void test_no_unused_references(void)
{
    std::string unused;
    for (const auto &entry : fs::directory_iterator(SCREENS_DIR))
    {
        const std::string name = entry.path().stem().string();
        if (checkedScreens.count(name) == 0)
        {
            if (updating)
            {
                fs::remove(entry.path());
            }
            else
            {
                unused += " " + name;
            }
        }
    }
    TEST_ASSERT_TRUE_MESSAGE(unused.empty(), ("unused reference images:" + unused).c_str());
}

int main(void)
{
    updating = getenv("UPDATE_SCREENSHOTS") != nullptr;
    fs::remove_all(FAILED_DIR);
    fs::create_directories(SCREENS_DIR);

    UNITY_BEGIN();
    RUN_TEST(test_scale);
    RUN_TEST(test_espresso);
    RUN_TEST(test_recipe_switcher);
    RUN_TEST(test_recipe_summary);
    RUN_TEST(test_recipe_config_ratio);
    RUN_TEST(test_recipe_config_weight);
    RUN_TEST(test_recipe_prepare);
    RUN_TEST(test_recipe_brewing);
    RUN_TEST(test_recipe_done);
    RUN_TEST(test_calibration);
    RUN_TEST(test_settings);
    RUN_TEST(test_mode_switcher);
    RUN_TEST(test_updater);
    RUN_TEST(test_no_unused_references);
    return UNITY_END();
}
