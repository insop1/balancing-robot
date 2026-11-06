#pragma once

#include <Arduino.h>
#include <cstdint>
#include "FastAccelStepper.h"

struct Motor {
    const uint8_t ms1;
    const uint8_t ms2;

    FastAccelStepper *stepper;

    uint8_t dir() const { return stepper->getDirectionPin(); }
    uint8_t step() const { return stepper->getStepPin(); }
};

namespace motors {

enum MSType {
    FULL,
    QUARTER,
    SIXTEENTH,
};

enum Side {
    LEFT,
    RIGHT,
};

Motor motor(uint8_t dir, uint8_t step, uint8_t ms1, uint8_t ms2);
void turn(Side side, MSType type, float w);
void init();
void turn(float degree);
void step(int steps);

}
