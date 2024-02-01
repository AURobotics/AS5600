#pragma once

// #include "Arduino.h"
#include "Wire.h"

#define baseAddress 0x36

class As5600
{
public:
    As5600(TwoWire *wire = &Wire);

    void magnetPresence();
    float rawAngle();
    void checkQuadrant();
}