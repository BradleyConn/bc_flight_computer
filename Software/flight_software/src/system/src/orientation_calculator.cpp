#include "../inc/orientation_calculator.h"
#include <stdio.h>

#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

OrientationCalculator::OrientationCalculator()
{
    _orientation_inst = Orientation();
}

OrientationCalculator::~OrientationCalculator()
{
}

// Orientation lib needs floats and radians
void OrientationCalculator::update(const bmi088DatasetConverted& data)
{
    // The gryo data is in milli degrees per second so convert to degrees per second
    auto x_deg_per_sec = data.gyro_data_converted.x_milli_degrees_per_sec / 1000.0f;
    auto y_deg_per_sec = data.gyro_data_converted.y_milli_degrees_per_sec / 1000.0f;
    auto z_deg_per_sec = data.gyro_data_converted.z_milli_degrees_per_sec / 1000.0f;
    // The orientation lib uses radians so convert to radians
    auto x_rad_per_sec = x_deg_per_sec * DEG_TO_RAD;
    auto y_rad_per_sec = y_deg_per_sec * DEG_TO_RAD;
    auto z_rad_per_sec = z_deg_per_sec * DEG_TO_RAD;

    // The reference frame is not what out sensor layout is so convert it
    // Also the sensor is mounted upside down so it basically did a 180 around the z axis. Which means the x and y are both reversed so negate them
    EulerAngles gyroMeasure;
    // yaw is z the z axis
    gyroMeasure.yaw = z_rad_per_sec;
    // pitch is the y axis, but the sensor is mounted upside down so negate it
    gyroMeasure.pitch = -y_rad_per_sec;
    // roll is the x axis, but the sensor is mounted upside down so negate it
    gyroMeasure.roll = -x_rad_per_sec;
    // Given odr calcuate dt
    float dt = 1.0f / data.gyro_odr;
    // Update the orientation
    _orientation_inst.update(gyroMeasure, dt);
    _euler = _orientation_inst.toEuler();

    static int count = 0;
    if (count++ % 100 == 0) {
        //printf("yaw = %f, pitch = %f, roll = %f\n", _euler.yaw * RAD_TO_DEG, _euler.pitch * RAD_TO_DEG, _euler.roll * RAD_TO_DEG);
        printf("y%fyp%fpr%fr\n", _euler.yaw * RAD_TO_DEG, _euler.pitch * RAD_TO_DEG, _euler.roll * RAD_TO_DEG);
    }

}

// Orientation lib needs floats
void OrientationCalculator::update_gravity(const bmi088DatasetConverted& data)
{
    // alpha should be roughly 0.02
    // The accelerometer data is in milli g's so convert to g's
    auto x_g = data.accel_data_converted.x_mg / 1000.0f;
    auto y_g = data.accel_data_converted.y_mg / 1000.0f;
    auto z_g = data.accel_data_converted.z_mg / 1000.0f;

    // The sensor is mounted upside down. As mentioned above it's basically a 180 around the z axis. Which means the x and y are both reversed so negate them
    _orientation_inst.updateGravity(-x_g, -y_g, z_g);
    _orientation_inst.applyComplementary(_orientation_inst.worldGravity, 0.02);
    _orientation_inst.zeroRoll();
    _euler = _orientation_inst.toEuler();

    static int count = 0;
    if (count++ % 1000 == 0) {
        //printf("x_g = %f, y_g = %f, z_g = %f\n", x_g, y_g, z_g);
        //printf("yaw = %f, pitch = %f, roll = %f\n", _euler.yaw * RAD_TO_DEG, _euler.pitch * RAD_TO_DEG, _euler.roll * RAD_TO_DEG);
    }

}

EulerAngles OrientationCalculator::getEulerYawPitchRollAngles() const
{
    return _euler;
}
