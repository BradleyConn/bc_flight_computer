#include "../../inc/controls/torque_to_tvc_angle.h"
#include "../../system/inc/thrust_curves/thrust_curve_E9.h"
#include <math.h>

TorqueToTvcAngle::TorqueToTvcAngle()
{
    //instantiate the thrust curve
    thrust_curve = std::make_unique<ThrustCurveE9>();
}

TorqueToTvcAngle::~TorqueToTvcAngle()
{
}
float TorqueToTvcAngle::getTvcAngle(float desired_PID_angle, uint32_t time_under_thrust_ms, float lever_arm_m)
{
    auto thrust_mN = thrust_curve->get_thrust_mN(time_under_thrust_ms);
    if (thrust_mN <= 5) {
        return 0;
    }
    auto thrust_N = (float)thrust_mN / 1000;
    // get the angle in radians

    auto desired_rads = desired_PID_angle * M_PI / 180;
    auto sin_input = desired_rads / (thrust_N * lever_arm_m);
    // make sure the result is in the range of -1 to 1
    if (sin_input > 1) {
        sin_input = 1;
    } else if (sin_input < -1) {
        sin_input = -1;
    }
    auto angle = asin(sin_input) * 180 / M_PI;

    return angle;
}
