#include "Arduino.h"
#include "control.h"
#include "esp32-hal.h"
#include "shared.h"
#include "log.h"
#include "gyro.h"
#include "motors.h"
#include <cmath>
#include <cstdint>

namespace {

uint32_t logPrev = 0;
constexpr uint32_t logInterval = 200;

Eigen::Vector3f prevW;
Eigen::Vector3f prevA;

unsigned int prevMs = 0;
constexpr float r = 0.13;

// dt should only have 3 decimal digits
constexpr float dt = 0.005;
const uint32_t msDelay = uint32_t(dt * 1000);

const float thresholdTheta = M_PI/1600;
float integralW = 0;
bool integralStarted = false;

} // namespace

int sign(float n) {
    if(n >= 0) {
        return 1;
    } else {
        return -1;
    }
}

void controlTask(void*) {
    TickType_t lastTick = xTaskGetTickCount();

    if(!gyro::init()) {
        net::log("Could not initialize imu");
        while(true) delay(100000);
    }

    motors::init();

    while(true) {
        unsigned int ms = millis();

        Command cmd;
        while(xQueueReceive(toControlQueue, &cmd, 0)) {
            if(cmd.type == ARMED) {
                motors::turn(motors::LEFT, motors::MSType::FULL, cmd.a);
            }
        }

        gyro::read();

        Eigen::Vector3f w = gyro::getGyro();
        Eigen::Vector3f a = gyro::getAccel();
        float thetaA = atan(a.z() / a.y());

        float dw_x = (w.x() - prevW.x());

        float alpha_xdt = dw_x / dt;
        // float alpha_xr = a.z() / r;
        // float w2 = sign(a.z()) * sqrt(abs(alpha_xr));

        if(abs(thetaA) < thresholdTheta) {
            integralStarted = true;
            integralW = 0;
        } else {
            if(integralStarted) {
                integralW += thresholdTheta;
                integralStarted = false;
            }
            integralW += w.x() * dt;
        }

        if(ms - logPrev > logInterval) {
            net::log("thet_a: %f thet_x: %f wx: %f alpha_xdt: %f", thetaA, integralW, w.x(), alpha_xdt);
            logPrev = millis();
        }

        prevW = w;
        prevA = a;
        vTaskDelayUntil(&lastTick, pdMS_TO_TICKS(msDelay));
    }
}
