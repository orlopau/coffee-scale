#ifndef NATIVE

#pragma once
#include "loadcell.h"

class HX711LoadCell : public LoadCell 
{
public:
    HX711LoadCell(int dout, int sck);
    void begin();
    bool isReady();
    long read();

private:
    uint8_t sck, dout;
};

#endif