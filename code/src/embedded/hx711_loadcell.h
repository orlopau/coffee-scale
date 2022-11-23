#ifndef NATIVE

#include "loadcell.h"
#include "HX711.h"

class HX711LoadCell : public LoadCell
{
public:
    HX711LoadCell(int dout, int sck, float scale = 1);
    void begin();
    void update();
    float getWeight();
    bool isNewWeight();
    void tare();
    void setScale(float scale);

private:
    int dout;
    int sck;

    HX711 hx711;
    long sumWeights = 0;
    byte index = 0;
    bool newWeight = false;
    long weight = 0;
    unsigned long lastUpdate = 0;
};

#endif