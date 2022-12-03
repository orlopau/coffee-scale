#ifndef NATIVE

#include "loadcell.h"

class HX711LoadCell : public LoadCell
{
public:
    HX711LoadCell(int dout, int sck, float scale = 1);
    void begin();
    void update();
    bool isReady();
    long read();
    float getWeight();
    bool isNewWeight();
    void tare();
    void setScale(float scale);

private:
    float scale;
    long lastWeight;
    uint8_t sck, dout;

    bool newWeight = false;
    float offset = 0;
};

#endif