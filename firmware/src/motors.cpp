#include "motors.h"
#include <cmath>
#include <FastAccelStepperEngine.h>

// WIP: Some ideas may be scratched
namespace {

FastAccelStepperEngine engine = FastAccelStepperEngine();

// Indexed by motors::Side
// LEFT = 0, RIGHT = 1
Motor motorList[2] = {
    // ms1 (tied to ms3), ms2
    {21, 19, nullptr}, 
    {5, 18, nullptr},
};

}

namespace motors {

Motor& get(Side side) {
    return motorList[(int)side];
}

void init() {
    engine.init();

    Motor &left = get(LEFT);
    Motor &right = get(RIGHT);

    left.stepper = engine.stepperConnectToPin(23);
    left.stepper->setDirectionPin(22);

    right.stepper = engine.stepperConnectToPin(2);
    right.stepper->setDirectionPin(15);

    for(Motor &mot : motorList) {
        mot.stepper->setSpeedInHz(400);
        mot.stepper->setAcceleration(400);

        pinMode(mot.ms2, OUTPUT);
        pinMode(mot.ms1, OUTPUT);

        digitalWrite(mot.ms1, LOW);
        digitalWrite(mot.ms2, LOW);
    }
}

// takes rad/s^2
void moveByAcceleration(Side side, float accel) {
    Motor &mot = get(side);
    float stepsAccel = ((100 * accel) / M_PI);
    mot.stepper->moveByAcceleration(stepsAccel);
}

void forceStop(Side side) {
    Motor &mot = get(side);
    mot.stepper->forceStop();
}

// takes rad/s
void moveByVelocity(Side side, float w) {
    Motor &mot = get(side);
    if(w == 0) {
        mot.stepper->setSpeedInHz(0);
        mot.stepper->stopMove();
        return;
    }

    float speed = ((100 * std::abs(w)) / M_PI);
    mot.stepper->setSpeedInHz(speed);

    if(w > 0) {
        mot.stepper->runForward();
    } else {
        mot.stepper->runBackward();
    }
}

}
