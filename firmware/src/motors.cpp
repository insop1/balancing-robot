#include "motors.h"
#include <cmath>
#include "esp32-hal-gpio.h"
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

    left.stepper->setSpeedInHz(400);
    left.stepper->setAcceleration(100000);

    pinMode(left.ms2, OUTPUT);
    pinMode(left.ms1, OUTPUT);

    right.stepper = engine.stepperConnectToPin(2);
    right.stepper->setDirectionPin(15);

    right.stepper->setSpeedInHz(400);
    right.stepper->setAcceleration(100000);
    
    pinMode(right.ms1, OUTPUT);
    pinMode(right.ms2, OUTPUT);

}

void turn(Side side, MSType type, float w) {
    Motor &mot = get(side);
    if(w == 0) {
        mot.stepper->setSpeedInHz(0);
        mot.stepper->stopMove();
        return;
    }

    switch(type) {
        case FULL:
            digitalWrite(mot.ms1, LOW);
            digitalWrite(mot.ms2, LOW);
            break;
        case QUARTER:
            digitalWrite(mot.ms1, LOW);
            digitalWrite(mot.ms2, HIGH);
            break;
        case SIXTEENTH:
            digitalWrite(mot.ms1, HIGH);
            digitalWrite(mot.ms2, HIGH);
            break;
    }

    float speed = (100 * abs(w)) / M_PI;

    mot.stepper->setSpeedInHz(speed);

    if(w > 0) {
        mot.stepper->runForward();
    } else {
        mot.stepper->runBackward();
    }
}

}
