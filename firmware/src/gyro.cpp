#include "gyro.h"
#include <cmath>

namespace {

bfs::Mpu6500 imu(&SPI, gyro::NCS);

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

    float rads = M_PI / 180;

    // Adjusting based on how the MPU6500 sits in the robot.
    float wx = z * cos(67.7 * rads) + y * cos(22.3 * rads);
    float wy = -x;
    float wz = -(z * sin(67.7 * rads)) + y * sin(22.3 * rads);
    
    return Eigen::Vector3f(wx, wy, wz);
}
Eigen::Vector3f getAccel() {
    float x = imu.accel_x_mps2();
    float y = imu.accel_y_mps2();
    float z = imu.accel_z_mps2();

    float rads = M_PI / 180;

    // Adjusting based on how the MPU6500 sits in the robot.
    float ax = z * cos(67.7 * rads) + y * cos(22.3 * rads);
    float ay = -x;
    float az = -(z * sin(67.7 * rads)) + y * sin(22.3 * rads);

    return Eigen::Vector3f(ax, ay, az);
}
float getTempC() {
    return imu.die_temp_c();
}

} // namespace
