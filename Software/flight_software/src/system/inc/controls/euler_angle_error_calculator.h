#ifndef EULER_ANGLE_ERROR_CALCULATOR_H
#define EULER_ANGLE_ERROR_CALCULATOR_H

#include "Orientation/Orientation/Orientation.h"

// Roll pitch roll could be calculated but as there is no roll controll the extra roll is not used therefor don't waste time calculating it
// This represents the offset or error from the desired vertical orientation.
// The pitch is the magnitude of the error and the roll is how much the rocket would need to roll before pitching forward to correct the error
struct AxisAngleErrors {
    // I probably want to convert this back to an int
    // TODO: radians or degrees?
    float yaw_Z_axis_servo_A_error_degrees;
    float pitch_y_axis_servo_C_error_degrees;
};

// NOTE: This is all in radians
AxisAngleErrors convertEulerAnglesToAxisError(const EulerAngles& euler);

#endif // EULER_ANGLE_ERROR_CALCULATOR_H
