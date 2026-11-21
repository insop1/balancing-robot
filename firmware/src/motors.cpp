#include "motors.h"
#include <cmath>
#include <FastAccelStepperEngine.h>

// WIP: Some ideas may be scratched
namespace {

FastAccelStepperEngine engine = FastAccelStepperEngine();

constexpr uint32_t MAX_SPEED = 2000;
constexpr uint32_t MAX_ACCEL = 1500;

// Indexed by motors::Side
// LEFT = 0, RIGHT = 1
Motor motorList[2] = {
    // ms1 (tied to ms3), ms2
    {0xFF, 0xFF, nullptr}, 
    {0xFF, 0xFF, nullptr},
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

    right.stepper = engine.stepperConnectToPin(16);
    right.stepper->setDirectionPin(4);

    for(Motor &mot : motorList) {
        mot.stepper->setSpeedInHz(MAX_SPEED);
        mot.stepper->setAcceleration(MAX_ACCEL);
    }
}

// takes rad/s^2
void moveByAcceleration(Side side, float accel) {
    Motor &mot = get(side);
    float stepsAccel = ((100.0f * accel) / M_PI);
    mot.stepper->setSpeedInHz(MAX_SPEED);
    mot.stepper->moveByAcceleration(stepsAccel);
}

void stopMove(Side side) {
    Motor &mot = get(side);
    mot.stepper->stopMove();
}

// takes rad/s
void moveByVelocity(Side side, float vel) {
    Motor &mot = get(side);

    // You can't set speed to 0
    // Using MilliHz because float truncates when cast to uint32_t
    uint32_t speedMilliHz = ((100.0f * std::abs(vel)) / M_PI) * 1000.0f;
    if(speedMilliHz == 0) {
        mot.stepper->stopMove();
        return;
    }
    mot.stepper->setSpeedInMilliHz(speedMilliHz);

    if(vel > 0) {
        mot.stepper->runForward();
    } else {
        mot.stepper->runBackward();
    }
}

}
