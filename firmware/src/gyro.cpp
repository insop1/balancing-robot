#include "gyro.h"

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
    float gx = imu.gyro_x_radps();
    float gy = imu.gyro_y_radps();
    float gz = imu.gyro_z_radps();
    
    return Eigen::Vector3f(gx, gy, gz);
}
Eigen::Vector3f getAccel() {
    float ax = imu.accel_x_mps2();
    float ay = imu.accel_y_mps2();
    float az = imu.accel_z_mps2();

    return Eigen::Vector3f(ax, ay, az);
}
float getTempC() {
    return imu.die_temp_c();
}

} // namespace
