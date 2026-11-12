#include "gyro.h"
#include <cmath>

namespace {

bfs::Mpu6500 imu(&SPI, gyro::NCS);
constexpr float rads = M_PI / 180;

Eigen::Vector3f adjustData(const Eigen::Vector3f &vec) {
    // Adjusting based on how the esp32 sits in the robot.
    float x = vec.z() * cos(67.7 * rads) + vec.y() * cos(22.3 * rads);
    float y = -vec.x();
    float z = -(vec.z() * sin(67.7 * rads)) + vec.y() * sin(22.3 * rads);

    return Eigen::Vector3f(x, y, z);
}

} // namespace

namespace gyro {

bool init() {
    SPI.begin(CLK, MISO, MOSI, NCS);
    return imu.Begin();
}
bool read() {
    return imu.Read();
}
Eigen::Vector3f getGyro() {
    float x = imu.gyro_x_radps();
    float y = imu.gyro_y_radps();
    float z = imu.gyro_z_radps();

    Eigen::Vector3f gyro(x, y, z);
    
    return adjustData(gyro);
}
Eigen::Vector3f getAccel() {
    float x = imu.accel_x_mps2();
    float y = imu.accel_y_mps2();
    float z = imu.accel_z_mps2();

    Eigen::Vector3f acc(x, y, z);

    return adjustData(acc);
}
float getTempC() {
    return imu.die_temp_c();
}

} // namespace
