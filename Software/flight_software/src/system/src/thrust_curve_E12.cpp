#include "thrust_curve_E12.h"
#include <stdint.h>

struct CurveAndIndex {
    uint8_t curve;
    uint32_t index;
};

struct ThrustDataUint32 {
    uint32_t t_mS; // time in milliseconds
    uint32_t f_mN; // thrust in millinewtons
};

//This is the data for an Estes E12 motor
// Use two thrust curves because the initial thrust changes rapidly
// massage the data and interpolate the curve to get more data points

// There's no need to include the time but it's included for clarity
ThrustDataUint32 thrustCurve_0_to_400_ms_by_25ms[] = {
    // clang-format off
    {0, 0},
    {25, 2522},
    {50, 5045},
    {75, 7588},
    {100, 10479},
    {125, 14037},
    {150, 17596},
    {175, 21154},
    {200, 24668},
    {225, 27944},
    {250, 31305},
    {275, 32432},
    {300, 29910},
    {325, 22641},
    {350, 16493},
    {375, 14173},
    {400, 12973}
    // clang-format on
};

ThrustDataUint32 thrustCurve_400_to_2500_ms_by_100ms[] = {
    // clang-format off
    {400,  12973},
    {500,  11712},
    {600,  11171},
    {700,  10631},
    {800,  10090},
    {900,  9730},
    {1000, 9550},
    {1100, 9910},
    {1200, 9550},
    {1300, 9730},
    {1400, 9730},
    {1500, 9730},
    {1600, 9730},
    {1700, 9550},
    {1800, 9730},
    {1900, 9730},
    {2000, 9550},
    {2100, 9550},
    {2200, 9730},
    {2300, 9370},
    {2400, 5950},
    {2500, 0}
    // clang-format on
};

CurveAndIndex get_index(uint32_t time_ms)
{
    uint32_t index = 0;
    uint8_t curve = 0;
    if (time_ms < 400) {
        index = time_ms / 25;
        curve = 0;
    } else if (time_ms < 2500) {
        index = (time_ms - 400) / 100;
        curve = 1;
    } else {
        index = 0;
        curve = 0;
    }
    return { curve, index };
}

ThrustCurveE12::ThrustCurveE12()
{
}

ThrustCurveE12::~ThrustCurveE12()
{
}

uint32_t ThrustCurveE12::get_thrust_mN(uint32_t time_ms)
{
    CurveAndIndex curveAndIndex = get_index(time_ms);
    uint32_t thrust_mN = 0;
    if (curveAndIndex.curve == 0) {
        thrust_mN = thrustCurve_0_to_400_ms_by_25ms[curveAndIndex.index].f_mN;
    } else if (curveAndIndex.curve == 1) {
        thrust_mN = thrustCurve_400_to_2500_ms_by_100ms[curveAndIndex.index].f_mN;
    }
    return thrust_mN;
}