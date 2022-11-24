#include <string.h>

#include "unity.h"
#include "mode_manager.h"
#include "millis.h"

class MockMode : public Mode
{
public:
    MockMode() : updateCalled(false){};
    ~MockMode(){};
    void update()
    {
        updateCalled = true;
    };
    bool updateCalled;
};

class MockDisplay : public Display
{
public:
    void begin(){};
    void display(float weight, unsigned long time){};
    void promptText(const char *prompt, const char *subtext){};
    void centerText(const char *text, const uint8_t size)
    {
        this->lastText = strdup(text);
    };
    void text(const char *text){};
    void update(){};
    void clear() { lastText = strdup(""); };
    char *lastText;
};

MockDisplay *display;
ModeManager *modeManager;
Mode *modes[2];
MockMode *mockModes[2];
const char *names[] = {"mode1", "mode2"};

void setUp(void)
{
    display = new MockDisplay();
    modes[0] = new MockMode();
    modes[1] = new MockMode();
    mockModes[0] = (MockMode *)modes[0];
    mockModes[1] = (MockMode *)modes[1];
    modeManager = new ModeManager(modes, names, 2, (*display));
}

void tearDown(void)
{
    delete display;
    delete modeManager;
    delete modes[0];
    delete modes[1];
}

void test_mode_manager_displays_text_when_changing(void)
{
    modeManager->update(1);
    TEST_ASSERT_EQUAL_STRING("mode2", display->lastText);
    modeManager->update(-1);
    TEST_ASSERT_EQUAL_STRING("mode1", display->lastText);
}

void test_mode_manager_calls_update_on_current_mode(void)
{
    modeManager->update(0);
    TEST_ASSERT_TRUE(mockModes[0]->updateCalled);
}

void test_mode_manager_does_not_call_update_when_changing(void)
{
    modeManager->update(1);
    modeManager->update(-1);
    modeManager->update(1);
    TEST_ASSERT_FALSE(mockModes[0]->updateCalled);
    TEST_ASSERT_FALSE(mockModes[0]->updateCalled);
}

void test_mode_manager_changes_mode_after_delay(void)
{
    modeManager->update(0);
    TEST_ASSERT_TRUE(mockModes[0]->updateCalled);
    modeManager->update(1);
    TEST_ASSERT_FALSE(mockModes[1]->updateCalled);
    modeManager->update(0);
    TEST_ASSERT_FALSE(mockModes[1]->updateCalled);
    sleep_for(MODE_SWITCH_DELAY_MS + 100);
    modeManager->update(0);
    TEST_ASSERT_TRUE(mockModes[1]->updateCalled);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_mode_manager_displays_text_when_changing);
    RUN_TEST(test_mode_manager_calls_update_on_current_mode);
    RUN_TEST(test_mode_manager_does_not_call_update_when_changing);
    RUN_TEST(test_mode_manager_changes_mode_after_delay);
    UNITY_END();
}