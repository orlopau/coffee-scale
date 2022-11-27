#ifndef NATIVE

#include "loadcell.h"
#include "Q2HX711.h"

class HX711LoadCell : public LoadCell
{
public:
    HX711LoadCell(int dout, int sck, float scale = 1);
    void update();
    float getWeight();
    bool isNewWeight();
    void tare();
    void setScale(float scale);

private:
    Q2HX711 hx711;
    float scale;

    long sumWeights = 0;
    int index = 0;
    bool newWeight = false;
    float lastWeight = 0;
    float offset = 0;
};

#endif