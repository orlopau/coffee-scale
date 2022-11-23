#include "loadcell.h"
#include "user_interface.h"
#include <math.h>

class MockLoadCell : public LoadCell
{
public:
    void begin() {}
    void update() {}
    float getWeight() { return weight; }
    bool isNewWeight() { return newWeight; }
    void tare()
    {
        weight = 0;
    }
    void setScale(float scale) {}
    float weight = 0;
    bool newWeight = false;
};

class MockButtons : public UserInput
{
public:
    void update() {}
    int getEncoderTicks() { return encoderTicks; };
    void resetEncoderTicks() { encoderTicks = 0; };
    ClickType getEncoderClick() { return encoderClick; };
    ClickType getBootClick() { return bootClick; };
    ClickType encoderClick = ClickType::NONE;
    ClickType bootClick = ClickType::NONE;
    int encoderTicks = 0;
};

class MockDisplay : public Display
{
public:
    void begin() {}
    void update() {}
    void display(float weight, unsigned long time)
    {
        this->weight = weight;
        this->time = time;
    };
    void promptText(const char *prompt, const char *subtext){};
    void text(const char *text){};
    void clear()
    {
        weight = NAN;
        time = -1;
    };
    float weight = NAN;
    unsigned long time = -1;
};