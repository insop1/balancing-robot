#pragma once

#include <Arduino.h>
#include <cstdint>

namespace motors {

constexpr uint8_t DIR_P = 22;
constexpr uint8_t STEP_P = 23;

void init();
void turn(float degree);
void step(int steps);

}
