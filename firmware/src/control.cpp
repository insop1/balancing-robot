#include "Arduino.h"
#include "control.h"
#include "shared.h"
#include "log.h"
#include "gyro.h"
#include "motors.h"
#include <cmath>

namespace {

using Eigen::Vector3f;
using motors::MSType;

uint32_t logPrev = 0;
constexpr uint32_t logInterval = 200;

Vector3f prevGyro = Vector3f::Zero();
Vector3f prevAccel = Vector3f::Zero();

unsigned int prevMs = 0;

// dt should only have 3 decimal digits
constexpr float dt = 0.005;
const uint32_t msDelay = uint32_t(dt * 1000);

bool armed = true;


int sign(float n) {
    if(n >= 0) {
        return 1;
    } else {
        return -1;
    }
}

// Currently testing this one
const float thresholdTheta = M_PI/1600;
float tiltIntegral = 0;
bool integralStarted = false;

float k1 = 1.0f;
float k2 = 1.0f;
float k3 = 1.0f;
float k4 = 1.0f;

float calcVel(const Vector3f &accel, const Vector3f &gyro) {
    float thetaA = atan(accel.z() / accel.y());

    float dw_x = (gyro.x() - prevGyro.x());
    float tiltRate = gyro.x();

    float alpha = dw_x / dt;

    if(std::abs(thetaA) < thresholdTheta) {
        integralStarted = true;
        tiltIntegral = 0;
    } else {
        if(integralStarted) {
            tiltIntegral += sign(thetaA) * thresholdTheta;
            integralStarted = false;
        }
        tiltIntegral += tiltRate * dt;
    }

    return -(k1 * tiltIntegral + k2 * thetaA + k3 * tiltRate + k4 * alpha);
}

// backup incase no time
float theta = 0;
bool thetaSeeded = false;
constexpr float filterK = 0.98f;
float kp = 150.0f;
float kd = 15.0f;

float calcAccel(const Vector3f &accel, const Vector3f &gyro) {
    float thetaA = atan(accel.z() / accel.y());
    float tiltRate = gyro.x();

    if(!thetaSeeded) {
        theta = thetaA;
        thetaSeeded = true;
    }

    // complimentary filter
    theta = filterK * (theta + tiltRate * dt) + (1 - filterK) * thetaA;
    return -(kp * theta + kd * tiltRate);
}

constexpr float *tuneArray[] = {&k1, &k2, &k3, &k4, &kp, &kd};

} // namespace

void controlTask(void*) {
    TickType_t lastTick = xTaskGetTickCount();

    if(!gyro::init()) {
        net::log("Could not initialize imu");
        while(true) delay(100000);
    }

    motors::init();

    while(true) {
        Command cmd;
        while(xQueueReceive(toControlQueue, &cmd, 0)) {
            switch(cmd.type) {
                case ARMED:
                    armed = cmd.a;
                    break;
                case SET_GAIN: {
                    int i = int(cmd.a);
                    if(i > sizeof(tuneArray) / sizeof(float)) {
                        *tuneArray[i] = cmd.b;
                        net::log("k[%d] = %f", i, cmd.b);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        unsigned int ms = millis();

        gyro::read();
        Vector3f gyro = gyro::getGyro();
        Vector3f accel = gyro::getAccel();

        if(!armed || otaActive) {
            motors::turn(motors::LEFT, MSType::FULL, 0);
            motors::turn(motors::RIGHT, MSType::FULL, 0);
        } else {
            float vel = calcVel(accel, gyro);
            motors::turn(motors::LEFT, MSType::FULL, vel);
            motors::turn(motors::RIGHT, MSType::FULL, vel);
        }

        prevGyro = gyro;
        prevAccel = accel;
        vTaskDelayUntil(&lastTick, pdMS_TO_TICKS(msDelay));
    }
}
