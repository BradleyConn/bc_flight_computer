#include "thrust_curves/thrust_curve_E9.h"
#include "thrust_curves/IThrustCurve.h"
#include <stdint.h>
#include <stdio.h>

//This is the data for an Estes E9 motor
ThrustDataUint32 thrustCurve[] = {
    // clang-format off
    // Use ones instead of zeros to avoid any potential weird divide by zero math
    {  0,      1},
    {  39,  1623},
    {  87,  5360},
    { 135,  9746},
    { 178, 13971},
    { 216, 16810},
    { 269, 19470},
    { 299, 17141},
    { 329, 14292},
    { 364, 12427},
    { 413, 10802},
    { 507,  9989},
    { 649,  9421},
    { 810,  8852},
    {1138,  8771},
    {1178,  8527},
    {1457,  8527},
    {1579,  8608},
    {1809,  8608},
    {2044,  8527},
    {2284,  8608},
    {2499,  8689},
    {2597,  8608},
    {2710,  9014},
    {2803,  8689},
    {2891,  9096},
    {2959,  9258},
    {3008,  9339},
    // It tails off here but commanding a thrust of 0 is useless so just assume it's still going strong for another second in case the timing was off
    //{3023,  7227},
    //{3034,  4466},
    //{3049,  1623},
    //{3090,     0},
    // Now add a fake points to mark the end of the curve
    {4000,  9339},
    {4001,     1},
    {4002,     1}
    // clang-format on
};


thrustCurvePoints ThrustCurveE9::get_index(uint32_t time_ms)
{
    thrustCurvePoints points;
    //safety checks
    if (time_ms < thrustCurve[0].t_mS) {
        points.point1 = thrustCurve[0];
        points.point2 = thrustCurve[1];
        return points;
    }
    if (time_ms > thrustCurve[sizeof(thrustCurve) / sizeof(ThrustDataUint32) - 1].t_mS) {
        points.point1 = {thrustCurve[sizeof(thrustCurve) / sizeof(ThrustDataUint32) - 2]};
        points.point2 = thrustCurve[sizeof(thrustCurve) / sizeof(ThrustDataUint32) - 1];
        return points;
    }

    uint32_t index = 0;
    for (index = 0; index < sizeof(thrustCurve) / sizeof(ThrustDataUint32); index++) {
        if (thrustCurve[index].t_mS > time_ms) {
            break;
        }
    }
    points.point1 = thrustCurve[index - 1];
    points.point2 = thrustCurve[index];
    return points;
}

ThrustCurveE9::ThrustCurveE9()
{
}

ThrustCurveE9::~ThrustCurveE9()
{
}

//generate a test function that calls this function every 25ms and prints the result
void ThrustCurveE9::print_test()
{
    for (uint32_t time_ms = 0; time_ms < 5002; time_ms += 25) {
        uint32_t thrust_mN = get_thrust_mN(time_ms);
        printf("time: %d, thrust: %d\n", time_ms, thrust_mN);
    }
}

// This whole thing could be precomputed and greatly sped up but there should be plenty of time to compute it
uint32_t ThrustCurveE9::get_thrust_mN(uint32_t time_ms)
{
    auto points = get_index(time_ms);
    uint32_t thrust_mN = 0;
    // linear interpolation using uint64_t to avoid overflow
    thrust_mN = (uint32_t)((uint64_t)points.point1.f_mN * (points.point2.t_mS - time_ms) + (uint64_t)points.point2.f_mN * (time_ms - points.point1.t_mS)) / (points.point2.t_mS - points.point1.t_mS);
    return thrust_mN;
}
