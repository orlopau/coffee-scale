#include <string.h>

#include "unity.h"
#include "mode_manager.h"
#include "millis.h"
#include "mocks.h"
#include "mock/mock_interface.h"
#include "mock/mock_display.h"

class MockMode : public Mode
{
public:
    MockMode(const char *name) : updateCalled(false), name(name){};
    ~MockMode(){};
    void update()
    {
        updateCalled = true;
    };
    void render(Canvas &canvas) override {}
    bool canSwitchMode()
    {
        return switchable;
    };
    const char *getName()
    {
        return name;
    };
    bool updateCalled;
    const char *name;
    bool switchable = true;
};

ModeManager *modeManager;
Mode *modes[3];
MockMode *mockModes[3];

void setUp(void)
{
    Interface::reset();
    Display::reset();
    modes[0] = new MockMode("Mock Mode 1");
    modes[1] = new MockMode("Mock Mode 2");
    modes[2] = new MockMode("Mock Mode 3");
    mockModes[0] = (MockMode *)modes[0];
    mockModes[1] = (MockMode *)modes[1];
    mockModes[2] = (MockMode *)modes[2];
    modeManager = new ModeManager(modes, 3, Display::mockCanvas);
}

void tearDown(void)
{
    delete modeManager;
    delete modes[0];
    delete modes[1];
    delete modes[2];
}

void enterSelection()
{
    Interface::encoderClick = ClickType::LONG;
    modeManager->update();
    Interface::encoderClick = ClickType::NONE;
    modeManager->update();
}

void test_mode_manager_updates_current_mode_by_default(void)
{
    modeManager->update();
    TEST_ASSERT_TRUE(mockModes[0]->updateCalled);
}

void test_mode_manager_shows_current_mode_after_long_click(void)
{
    enterSelection();
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("Mock Mode 1"));
}

void test_mode_manager_shows_next_and_previous_mode_after_rotation(void)
{
    enterSelection();
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("Mock Mode 1"));

    Interface::encoderDirection = Interface::EncoderDirection::CW;
    modeManager->update();
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("Mock Mode 2"));

    Interface::encoderDirection = Interface::EncoderDirection::CCW;
    modeManager->update();
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("Mock Mode 1"));
}

void test_mode_manager_modes_lower_bound(void)
{
    enterSelection();
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("Mock Mode 1"));

    Interface::encoderDirection = Interface::EncoderDirection::CCW;
    modeManager->update();
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("Mock Mode 1"));
}

void test_mode_manager_modes_upper_bound(void)
{
    enterSelection();
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("Mock Mode 1"));

    Interface::encoderDirection = Interface::EncoderDirection::CW;
    modeManager->update();
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("Mock Mode 2"));

    Interface::encoderDirection = Interface::EncoderDirection::CW;
    modeManager->update();
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("Mock Mode 3"));

    Interface::encoderDirection = Interface::EncoderDirection::CW;
    modeManager->update();
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("Mock Mode 3"));
}

void test_mode_manager_does_not_call_update_when_changing(void)
{
    enterSelection();
    mockModes[0]->updateCalled = false;

    // some updates
    modeManager->update();
    modeManager->update();
    modeManager->update();

    TEST_ASSERT_FALSE(mockModes[0]->updateCalled);
    TEST_ASSERT_FALSE(mockModes[1]->updateCalled);
    TEST_ASSERT_FALSE(mockModes[2]->updateCalled);
}

void test_mode_manager_selects_mode_with_single_click(void)
{
    enterSelection();
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("Mock Mode 1"));

    Interface::encoderDirection = Interface::EncoderDirection::CW;
    modeManager->update();
    Interface::encoderDirection = Interface::EncoderDirection::NONE;
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("Mock Mode 2"));

    Interface::encoderClick = ClickType::SINGLE;
    modeManager->update();
    Interface::encoderClick = ClickType::NONE;
    modeManager->update();
    TEST_ASSERT_TRUE(mockModes[1]->updateCalled);
}

void test_mode_manager_can_only_switch_when_mode_allows_it(void)
{
    mockModes[0]->switchable = false;
    enterSelection();
    TEST_ASSERT_FALSE(Display::mockCanvas.hasText("Mock Mode 1"));
    TEST_ASSERT_TRUE(mockModes[0]->updateCalled);
}

void test_mode_manager_updates_next_mode_only_at_next_tick(void)
{
    enterSelection();
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("Mock Mode 1"));

    Interface::encoderDirection = Interface::EncoderDirection::CW;
    modeManager->update();
    Interface::encoderDirection = Interface::EncoderDirection::NONE;
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("Mock Mode 2"));

    Interface::encoderClick = ClickType::SINGLE;
    modeManager->update();
    TEST_ASSERT_FALSE(mockModes[1]->updateCalled);
    modeManager->update();
    TEST_ASSERT_TRUE(mockModes[1]->updateCalled);
}

class MockCanvasMode : public MockMode
{
public:
    MockCanvasMode(const char *name) : MockMode(name){};
    void render(Canvas &canvas) override
    {
        renderCount++;
        canvas.drawText(0, 10, name);
    }
    int renderCount = 0;
};

void test_mode_manager_renders_canvas_mode_after_update(void)
{
    RecordingCanvas canvas;
    MockCanvasMode mode("Canvas Mode");
    Mode *canvasModes[] = {&mode};
    ModeManager manager(canvasModes, 1, canvas);
    manager.begin();

    manager.update();
    TEST_ASSERT_TRUE(mode.updateCalled);
    TEST_ASSERT_EQUAL(1, mode.renderCount);
    TEST_ASSERT_EQUAL(1, canvas.clears);
    TEST_ASSERT_EQUAL(1, canvas.flushes);
    TEST_ASSERT_TRUE(canvas.hasText("Canvas Mode"));
}

void test_mode_manager_caps_render_rate(void)
{
    RecordingCanvas canvas;
    MockCanvasMode mode("Canvas Mode");
    Mode *canvasModes[] = {&mode};
    ModeManager manager(canvasModes, 1, canvas);
    manager.begin();

    // many updates in quick succession render only one frame
    for (int i = 0; i < 20; i++)
    {
        manager.update();
    }
    TEST_ASSERT_EQUAL(1, mode.renderCount);

    // after the frame interval passed, the next update renders again
    sleep_for(RENDER_INTERVAL_MS + 5);
    manager.update();
    TEST_ASSERT_EQUAL(2, mode.renderCount);
}

void test_mode_manager_renders_immediately_after_mode_selection(void)
{
    RecordingCanvas canvas;
    MockCanvasMode first("First"), second("Second");
    Mode *canvasModes[] = {&first, &second};
    ModeManager manager(canvasModes, 2, canvas);
    manager.begin();
    manager.update();
    TEST_ASSERT_EQUAL(1, first.renderCount);

    // switch to the second mode within the same frame interval
    Interface::encoderClick = ClickType::LONG;
    manager.update();
    Interface::encoderClick = ClickType::NONE;
    Interface::encoderDirection = Interface::EncoderDirection::CW;
    manager.update();
    Interface::encoderDirection = Interface::EncoderDirection::NONE;
    Interface::encoderClick = ClickType::SINGLE;
    manager.update();
    Interface::encoderClick = ClickType::NONE;

    manager.update();
    TEST_ASSERT_EQUAL(1, second.renderCount);
    TEST_ASSERT_TRUE(canvas.hasText("Second"));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_mode_manager_renders_canvas_mode_after_update);
    RUN_TEST(test_mode_manager_caps_render_rate);
    RUN_TEST(test_mode_manager_renders_immediately_after_mode_selection);
    RUN_TEST(test_mode_manager_updates_current_mode_by_default);
    RUN_TEST(test_mode_manager_shows_current_mode_after_long_click);
    RUN_TEST(test_mode_manager_shows_next_and_previous_mode_after_rotation);
    RUN_TEST(test_mode_manager_modes_lower_bound);
    RUN_TEST(test_mode_manager_modes_upper_bound);
    RUN_TEST(test_mode_manager_does_not_call_update_when_changing);
    RUN_TEST(test_mode_manager_selects_mode_with_single_click);
    RUN_TEST(test_mode_manager_can_only_switch_when_mode_allows_it);
    RUN_TEST(test_mode_manager_updates_next_mode_only_at_next_tick);
    UNITY_END();
}