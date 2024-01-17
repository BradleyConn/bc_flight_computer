#pragma once
#include <stdint.h>

struct ThrustDataUint32 {
    uint32_t t_mS; // time in milliseconds
    uint32_t f_mN; // thrust in millinewtons
};

//two points
struct thrustCurvePoints {
    ThrustDataUint32 point1;
    ThrustDataUint32 point2;
};

class IThrustCurve
{
public:
    virtual uint32_t get_thrust_mN(uint32_t time_ms) = 0;
    virtual void print_test() = 0;
};