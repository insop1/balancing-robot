#include "control.h"
#include "shared.h"
#include "log.h"
#include "gyro.h"
#include "motors.h"
#include "util.h"

#include <Arduino.h>
#include <cmath>

namespace {

using Eigen::Vector3f;
using Eigen::Vector2f;

uint32_t logPrev = 0;
constexpr uint32_t LOG_INTERVAL_MS = 200;

uint32_t statusPrev = 0;
constexpr uint32_t STATUS_INTERVAL_MS = 500;

bool armed = false;

constexpr uint32_t LOOP_MS = 5;
constexpr float dt = LOOP_MS / 1000.0f;

Vector3f prev2Gyro = Vector3f::Zero();
Vector3f prevGyro = Vector3f::Zero();
Vector3f prevAccel = Vector3f::Zero();

// Currently testing this one
constexpr float IMU_TILT_OFFSET = 2 * (M_PI/180);
constexpr float THRESHOLD_ANGLE = 1.8 * (M_PI/180);

float tiltIntegral = 0;
float tiltIntInt = 0;
bool integralStarted = false;

float k1 = 0.0f;
float k2 = 80.0f;
float k3 = 40.0f;
float k4 = 0.0f;
float k5 = 120.0f;

float calcBalVel(const Vector3f &accel, const Vector3f &gyro, float ms) {
    float thetaA = atan(accel.z() / accel.y());

    float dGyroX = (gyro.x() - prevGyro.x());
    float dPrevGyroX = (prevGyro.x() - prev2Gyro.x());
    float tiltRate = gyro.x();

    float alpha = dGyroX / dt;
    float prevAlpha = dPrevGyroX / dt;
    float avgAlpha = (alpha + prevAlpha) / 2;

    if(std::abs(thetaA - IMU_TILT_OFFSET) < THRESHOLD_ANGLE) {
        integralStarted = true;
        tiltIntegral = 0;
        tiltIntInt = 0;
    } else {
        if(integralStarted) {
            tiltIntegral += util::sign(thetaA - IMU_TILT_OFFSET) * THRESHOLD_ANGLE;
            integralStarted = false;
        }
        tiltIntegral += tiltRate * dt;
        tiltIntInt += tiltIntegral * dt;
    }

    float result = (k1 * (tiltIntegral - IMU_TILT_OFFSET) + k2 * (thetaA - IMU_TILT_OFFSET) + k3 * tiltRate + k4 * avgAlpha + k5 * tiltIntInt);

    // float compAngle = 0.2 * thetaA + 0.8 * tiltIntegral;
    // if(std::abs(compAngle) < THRESHOLD_ANGLE) {
    //     result = 0;
    // }

    if(ms - logPrev > LOG_INTERVAL_MS) {
        net::log(
            "\n"
            "tiltInt: %f tiltIntInt: %f\n"
            "thetaA: %f tiltRate: %f alpha: %f result: %f\n", 
            tiltIntegral - IMU_TILT_OFFSET, tiltIntInt, thetaA - IMU_TILT_OFFSET, tiltRate, avgAlpha, result
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
                tiltIntegral = 0;
                tiltIntInt = 0;
                integralStarted = false;
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
            vel = util::clamp(vel, -40, 40);
            motors::moveByVelocity(motors::LEFT, vel);
            motors::moveByVelocity(motors::RIGHT, vel);
        }

        if(ms - statusPrev > STATUS_INTERVAL_MS) {
            net::status(
                "{"
                "\"armed\":%d,"
                "\"ota\":%d"
                "}", 
                armed, otaActive.load()
            );
            statusPrev = ms;
        }

        prev2Gyro = prevGyro;
        prevGyro = gyro;
        prevAccel = accel;
        vTaskDelayUntil(&lastTick, pdMS_TO_TICKS(LOOP_MS));
    }
}
