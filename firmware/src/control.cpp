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

constexpr uint8_t BTN_PIN = 36;
uint32_t btnPrev = 0;
constexpr uint32_t BTN_COOLDOWN_MS = 200;

uint32_t logPrev = 0;
constexpr uint32_t LOG_INTERVAL_MS = 200;

// Keep STATUS_INTERVAL_MS matching with webserver/browser/src/hook/RobotStatus.ts
uint32_t statusPrev = 0;
constexpr uint32_t STATUS_INTERVAL_MS = 500;

bool armed = false;

constexpr uint32_t LOOP_MS = 5;
constexpr float dt = LOOP_MS / 1000.0f;

Vector3f prev2Gyro = Vector3f::Zero();
Vector3f prevGyro = Vector3f::Zero();
Vector3f prevAccel = Vector3f::Zero();

// Currently testing this one
constexpr float IMU_TILT_OFFSET = 1.5 * (M_PI/180);
constexpr float THRESHOLD_ANGLE = 1.8 * (M_PI/180);

float tiltIntegral = 0;
float tiltIntInt = 0;
bool integralStarted = false;

float k1 = 10.0f;
float k2 = 60.0f;
float k3 = 15.0f;
float k4 = 0.5f;
float k5 = 40.0f;
float k6 = 0.0f;

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

    // if(ms - logPrev > LOG_INTERVAL_MS) {
    //     net::log(
    //         "\n"
    //         "tiltInt: %f tiltIntInt: %f\n"
    //         "thetaA: %f tiltRate: %f alpha: %f result: %f\n", 
    //         tiltIntegral - IMU_TILT_OFFSET, tiltIntInt, thetaA - IMU_TILT_OFFSET, tiltRate, avgAlpha, result
    //     );
    //     logPrev = ms;
    // }

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

constexpr float *tuneArray[] = {&k1, &k2, &k3, &k4, &k5, &k6, &kp, &kd};

// values are -1 to 1 for x and y
Vector2f joystick = Vector2f::Zero();

void handleCommand(const Command &cmd) {
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

void handleQueue() {
    Command cmd;
    while(xQueueReceive(toControlQueue, &cmd, 0)) {
        handleCommand(cmd);
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

    pinMode(BTN_PIN, INPUT_PULLDOWN);

    motors::init();
    net::log("Control system initiatized");

    while(true) {
        handleQueue();
        unsigned int ms = millis();

        if(digitalRead(BTN_PIN) == HIGH && ms - btnPrev > BTN_COOLDOWN_MS) {
            armed = !armed;
            if(armed) {
                net::log("Robot armed");
            } else {
                net::log("Robot disarmed");
            }
            btnPrev = ms;
        }

        gyro::read();
        Vector3f gyro = gyro::getGyro();
        Vector3f accel = gyro::getAccel();

        float turnRate = gyro.y();

        if(!armed || otaActive) {
            motors::stopMove(motors::LEFT);
            motors::stopMove(motors::RIGHT);
        } else {
            float vel = -calcBalVel(accel, gyro, ms);
            vel = util::clamp(vel, -40, 40);

            float lVel = vel - k6 * turnRate;
            float rVel = vel + k6 * turnRate;
            motors::moveByVelocity(motors::LEFT, lVel);
            motors::moveByVelocity(motors::RIGHT, rVel);

            if(ms - logPrev > LOG_INTERVAL_MS) {
                logPrev = ms;
            }
        }

        if(ms - statusPrev > STATUS_INTERVAL_MS) {
            net::status(
                "{"
                "\"armed\":%s,"
                "\"ota\":%s"
                "}", 
                util::boolStr(armed), 
                util::boolStr(otaActive)
            );
            statusPrev = ms;
        }

        prev2Gyro = prevGyro;
        prevGyro = gyro;
        prevAccel = accel;
        vTaskDelayUntil(&lastTick, pdMS_TO_TICKS(LOOP_MS));
    }
}
