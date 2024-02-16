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

uint32_t TorqueToTvcAngle::getTvcAngle(uint32_t desiredTorque_mN, uint32_t time_ms)
{
    auto thrust = thrust_curve->get_thrust_mN(time_ms);
    // calculate the TVC angle using trig
    auto angle = asin(desiredTorque_mN / thrust);
    return angle;
}
