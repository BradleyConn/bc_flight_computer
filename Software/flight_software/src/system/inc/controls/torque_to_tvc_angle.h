#ifndef TORQUE_TO_TVC_ANGLE_H
#define TORQUE_TO_TVC_ANGLE_H

#include "../../inc/thrust_curves/IThrustCurve.h"
#include <memory>
#include <stdint.h>

// Define a class to calculate the TVC angle based on torque and time
class TorqueToTvcAngle
{
public:
    // TODO: Take the thrust curve as an argument
    // Constructor
    TorqueToTvcAngle();

    // Destructor
    ~TorqueToTvcAngle();

    // Function to get the TVC angle for a given torque and time
    uint32_t getTvcAngle(uint32_t desiredTorque_mN, uint32_t time_ms);

private:
    // Unique pointer to the thrust curve
    std::unique_ptr<IThrustCurve> thrust_curve;
};

#endif // TORQUE_TO_TVC_ANGLE_H
