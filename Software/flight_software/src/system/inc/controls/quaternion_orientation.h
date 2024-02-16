#ifndef QUATERNION_ORIENTATION_H
#define QUATERNION_ORIENTATION_H

#include "Orientation/Orientation/Orientation.h"
#include "drv_bmi088.h"

class QuaternionOrientation {
public:
    QuaternionOrientation();
    ~QuaternionOrientation();

    // Send in gyro data in DEGREES and get out euler angles in RADIANS (yaw, pitch, roll)
    void update(const bmi088DatasetConverted &data, float dt);
    // This is to be used ON THE PAD ONLY to deduce the angle offset from vertical using the accelerometer
    // Call this for at least a second while on the pad to get a good calibration
    // Uses an alpha value of .98 so it needs to be called a lot to settle out
    // Can be stopped or left on until launch is detected
    void update_gravity(const bmi088DatasetConverted &data, float dt);
    // Get the euler angles in RADIANS (yaw, pitch, roll)
    EulerAngles getEulerYawPitchRollAngles() const;

private:
    // Use the orientation lib to handle the quaternion math
    Orientation _orientation_inst;
    EulerAngles _euler;
};

#endif // QUATERNION_ORIENTATION_H
