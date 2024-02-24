#ifndef ORIENTATION_CALCULATOR_H
#define ORIENTATION_CALCULATOR_H

#include "Orientation/Orientation/Orientation.h"
#include "drv_bmi088.h"

class OrientationCalculator {
public:
    OrientationCalculator();
    ~OrientationCalculator();

    // Send in gyro data in DEGREES and get out euler angles in RADIANS (yaw, pitch, roll)
    // Calculating dt based on the odr in data
    void update(const bmi088DatasetConverted &data);
    // This is to be used ON THE PAD ONLY to deduce the angle offset from vertical using the accelerometer
    // Call this for at least a second while on the pad to get a good calibration
    // Uses an alpha value of .98 so it needs to be called a lot to settle out
    // Can be stopped or left on until launch is detected
    void update_gravity(const bmi088DatasetConverted &data);
    // Get the euler angles in RADIANS (yaw, pitch, roll)
    EulerAngles getEulerYawPitchRollAngles() const;
    // Get the quaternion values a, b, c, d. Make a struct to hold them
    struct QuaternionValues {
        float a;
        float b;
        float c;
        float d;
    };
    QuaternionValues getQuaternionValues() const
    {
        return {_orientation_inst.orientation.a, _orientation_inst.orientation.b, _orientation_inst.orientation.c, _orientation_inst.orientation.d};
    }

private:
    // Use the orientation lib to handle the quaternion math
    Orientation _orientation_inst;
    EulerAngles _euler;
};

#endif // ORIENTATION_CALCULATOR_H
