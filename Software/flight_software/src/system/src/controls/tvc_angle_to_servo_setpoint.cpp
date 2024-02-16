#include "../../inc/controls/tvc_angle_to_servo_setpoint.h"

TVCAngleToServoSetpoint::TVCAngleToServoSetpoint()
{
}

TVCAngleToServoSetpoint::~TVCAngleToServoSetpoint()
{
}

float TVCAngleToServoSetpoint::convertAngleToSetpoint(float tvc_angle_degrees)
{
    // The TVC mount only has a range of +- 5 degrees
    if (tvc_angle_degrees > 5.0f)
    {
        return 5.0f;
    }
    else if (tvc_angle_degrees < -5.0f)
    {
        return -5.0f;
    }

    // Now do the lookup
    // For now it's rougly a 6 to 1 ratio
    return tvc_angle_degrees * 6.0f;
}
