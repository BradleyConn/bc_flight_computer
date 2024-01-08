#ifndef THRUST_CURVE_E12_H
#define THRUST_CURVE_E12_H

#include "IThrustCurve.h"
#include <stdint.h>

class ThrustCurveE12 : public IThrustCurve
{
public:
    ThrustCurveE12();
    ~ThrustCurveE12();
    // time_ms is the delta between launch start and now in milliseconds
    // returns the thrust in millinewtons
    uint32_t get_thrust_mN(uint32_t time_ms) override;
};

#endif // THRUST_CURVE_E12_H
