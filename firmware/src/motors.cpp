#include "motors.h"
#include <cmath>
#include "esp32-hal-gpio.h"
#include "log.h"
#include <FastAccelStepperEngine.h>


// WIP: Some ideas may be scratched

namespace {

template <typename T>
constexpr int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

FastAccelStepperEngine engine = FastAccelStepperEngine();

// ms1&ms3, ms2
Motor left{19, 21, nullptr};
Motor right{18, 5, nullptr};

}

namespace motors {

void init() {
    engine.init();

    left.stepper = engine.stepperConnectToPin(23);
    left.stepper->setDirectionPin(22);

    right.stepper = engine.stepperConnectToPin(15);
    right.stepper->setDirectionPin(2);

    left.stepper->setSpeedInHz(400);
    left.stepper->setAcceleration(100000);

    right.stepper->setSpeedInHz(400);
    right.stepper->setAcceleration(100000);
}

// theta is in degrees - w is in rad / s
void turn(float deg, float w) {
    if(w > 0) {
        digitalWrite(left.dir(), HIGH);
    } else if (w < 0) {
        digitalWrite(left.dir(), LOW);
    } else {
        return;
    }

    float aDeg = abs(deg);
    float n1 = floor(aDeg / 1.8);
    float n2 = floor(4*(aDeg / 1.8 - n1));
    float n3 = floor(16*(aDeg / 1.8 - n1 - n2 / 4));
    
    float r = deg - 1.8 * sgn(deg) * (n1 - n2/4 - n3/16);
    float new_theta = 1.8 * (n1 - n2/4 - n3/16); 

    net::log("r: %f | new_theta: %f", r, new_theta);

    float speed = (100/M_PI) * abs(w);
    left.stepper->setSpeedInHz(speed);

    digitalWrite(left.ms1, LOW);
    digitalWrite(left.ms2, LOW);
    left.stepper->move(n1);

    left.stepper->setSpeedInHz(speed * 4);
    digitalWrite(left.ms2, HIGH);
    left.stepper->move(n2);

    left.stepper->setSpeedInHz(speed * 16);
    digitalWrite(left.ms1, HIGH);
    left.stepper->move(n3);
}

}
