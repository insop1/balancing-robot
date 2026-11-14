#include "Arduino.h"
#include "control.h"
#include "shared.h"
#include "log.h"
#include "gyro.h"
#include "motors.h"
#include "util.h"
#include <cmath>

namespace {

using Eigen::Vector3f;
using Eigen::Vector2f;

uint32_t logPrev = 0;
constexpr uint32_t logInterval = 200;

uint32_t statusPrev = 0;
constexpr uint32_t statusInterval = 500;

// dt should only have 3 decimal digits
constexpr float dt = 0.005f;
const uint32_t msDelay = uint32_t(dt * 1000);

bool armed = false;

Vector3f prev2Gyro = Vector3f::Zero();
Vector3f prevGyro = Vector3f::Zero();
Vector3f prevAccel = Vector3f::Zero();

// Currently testing this one
float placementOffset = 2 * (M_PI/180);
constexpr float thresholdAngle = 1.8 * (M_PI/180);

float tiltIntegral = 0;
float tiltIntInt = 0;
bool integralStarted = false;

float k1 = 0.0f;
float k2 = 120.0f;
float k3 = 40.0f;
float k4 = 0.0f;
float k5 = 80.0f;

float calcBalVel(const Vector3f &accel, const Vector3f &gyro, float ms) {
    float thetaA = atan(accel.z() / accel.y());

    float dGyroX = (gyro.x() - prevGyro.x());
    float dPrevGyroX = (prevGyro.x() - prev2Gyro.x());
    float tiltRate = gyro.x();

    float alpha = dGyroX / dt;
    float prevAlpha = dPrevGyroX / dt;
    float avgAlpha = (alpha + prevAlpha) / 2;

    if(std::abs(thetaA - placementOffset) < thresholdAngle) {
        integralStarted = true;
        tiltIntegral = 0;
        tiltIntInt = 0;
    } else {
        if(integralStarted) {
            tiltIntegral += util::sign(thetaA) * thresholdAngle;
            integralStarted = false;
        }
        tiltIntegral += tiltRate * dt;
        tiltIntInt += (tiltIntegral - placementOffset) * dt;
    }

    float result = (k1 * (tiltIntegral - placementOffset) + k2 * (thetaA - placementOffset) + k3 * tiltRate + k4 * avgAlpha + k5 * tiltIntInt);

    // float compAngle = 0.2 * thetaA + 0.8 * tiltIntegral;
    // if(std::abs(compAngle) < thresholdAngle) {
    //     result = 0;
    // }

    if(ms - logPrev > logInterval) {
        net::log(
            "\n"
            "tiltInt: %f tiltIntInt: %f\n"
            "thetaA: %f tiltRate: %f alpha: %f result: %f\n", 
            tiltIntegral - placementOffset, tiltIntInt, thetaA - placementOffset, tiltRate, avgAlpha, result
        );
        logPrev = ms;
    }

    return result;
}

// backup incase no time
float theta = 0;
bool thetaSeeded = false;
constexpr float filterK = 0.98f;
float kp = 400.0f;
float kd = 15.0f;

float calcBalAccel(const Vector3f &accel, const Vector3f &gyro) {
    float thetaA = atan(accel.z() / accel.y());
    float tiltRate = gyro.x();

    if(!thetaSeeded) {
        theta = thetaA;
        thetaSeeded = true;
    }

    // complimentary filter
    theta = filterK * (theta + tiltRate * dt) + (1 - filterK) * thetaA;
    return (kp * theta + kd * tiltRate);
}

constexpr float *tuneArray[] = {&k1, &k2, &k3, &k4, &k5, &kp, &kd};

// values are -1 to 1 for x and y
Vector2f joystick = Vector2f::Zero();

void handleNetworkQueue() {
    Command cmd;
    while(xQueueReceive(toControlQueue, &cmd, 0)) {
        switch(cmd.type) {
            case CommandType::Armed: {
                armed = cmd.a;
                if(armed) {
                    net::log("Robot armed");
                } else {
                    net::log("Robot disarmed");
                }
                break;
            }
            case CommandType::SetGain: {
                int i = int(cmd.a);
                if(i < sizeof(tuneArray) / sizeof(float)) {
                    *tuneArray[i] = cmd.b;
                    net::log("k[%d] = %f", i + 1, cmd.b);
                }
                break;
            }
            case CommandType::Joystick: {
                joystick = Vector2f(cmd.a, cmd.b);
                break;
            }
            default:
                break;
        }
    }
}

} // namespace

void controlTask(void*) {
    TickType_t lastTick = xTaskGetTickCount();

    if(!gyro::init()) {
        while(true) {
            net::log("Could not initialize imu");
            delay(1000);
        }
    }

    motors::init();
    net::log("Control system initiatized");

    while(true) {
        handleNetworkQueue();
        unsigned int ms = millis();

        gyro::read();
        Vector3f gyro = gyro::getGyro();
        Vector3f accel = gyro::getAccel();

        if(!armed || otaActive) {
            motors::stopMove(motors::LEFT);
            motors::stopMove(motors::RIGHT);
        } else {
            float vel = -calcBalVel(accel, gyro, ms);
            vel = util::clamp(vel, -120, 120);
            motors::moveByVelocity(motors::LEFT, vel);
            motors::moveByVelocity(motors::RIGHT, vel);
        }

        if(ms - statusPrev > statusInterval) {
            net::status(
                "{"
                "\"armed\":%d,"
                "\"ota\":%d"
                "}", 
                armed, otaActive.load()
            );
            statusPrev = ms;
        }

        prevGyro = gyro;
        prev2Gyro = prevGyro;
        prevAccel = accel;
        vTaskDelayUntil(&lastTick, pdMS_TO_TICKS(msDelay));
    }
}
