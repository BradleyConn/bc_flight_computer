#ifndef THRUST_CURVE_E9_H
#define THRUST_CURVE_E9_H

#include "IThrustCurve.h"
#include <stdint.h>

class ThrustCurveE9 : public IThrustCurve
{
public:
    ThrustCurveE9();
    ~ThrustCurveE9();
    // time_ms is the delta between launch start and now in milliseconds
    // returns the thrust in millinewtons
    uint32_t get_thrust_mN(uint32_t time_ms) override;

    void print_test() override;

private:
    thrustCurvePoints get_index(uint32_t time_ms);
};

#endif // THRUST_CURVE_E9_H
