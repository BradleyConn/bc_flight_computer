#ifndef TORQUE_TO_TVC_ANGLE_H
#define TORQUE_TO_TVC_ANGLE_H

#include <stdint.h>

// Define a class to calculate the TVC angle based on torque and time
class TorqueToTvcAngle
{
public:
    // Constructor
    TorqueToTvcAngle();

    // Destructor
    ~TorqueToTvcAngle();

    // Function to get the TVC angle for a given torque and time
    uint32_t getTvcAngle(uint32_t torque_mN, uint32_t time_ms);

private:
    uint32_t thrustCurve[1000];
};

#endif // TORQUE_TO_TVC_ANGLE_H
