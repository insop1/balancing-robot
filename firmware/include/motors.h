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

void moveByAcceleration(Side side, float accel);
void stopMove(Side side);
void moveByVelocity(Side side, float vel);
void init();
void step(int steps);

}
