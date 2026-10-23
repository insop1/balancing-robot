#include "motors.h"
#include <cmath>
#include "log.h"

namespace {

constexpr float MAXD = 1.8;

} // namespace

// Huge WIP

namespace motors {


void turn(float deg) {
    if(deg >= 0) {
        digitalWrite(DIR_P, HIGH);
    } else {
        digitalWrite(DIR_P, LOW);
    }

    float degabs = abs(deg);
    float n = abs(std::floor(deg / MAXD));
    step(n);
    float rem = deg - (1.8 * n);

    net::log("%f", rem);
}

void step(int steps) {
    for(int i = 0; i < steps; i++) {
        digitalWrite(STEP_P, LOW);
        delay(2);
        digitalWrite(STEP_P, HIGH);
        delay(10);
    }
}

} // namespace
