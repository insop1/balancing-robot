#pragma once

#include <Arduino.h>
#include <mpu6500.h>
#include <ArduinoEigenDense.h>

namespace gyro {

constexpr uint8_t NCS = 27;
constexpr uint8_t CLK = 14;
constexpr uint8_t MISO = 12;
constexpr uint8_t MOSI = 13;

bool init();
bool read();
Eigen::Vector3f getGyro();
Eigen::Vector3f getAccel();
float getTempC();

}
