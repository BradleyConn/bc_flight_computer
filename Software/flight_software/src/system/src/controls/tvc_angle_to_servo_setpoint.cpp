#include "../../inc/controls/tvc_angle_to_servo_setpoint.h"
#include <math.h>
#include <stdio.h>

TVCAngleToServoSetpoint::TVCAngleToServoSetpoint()
{
}

TVCAngleToServoSetpoint::~TVCAngleToServoSetpoint()
{
}

struct TVCToServo {
    float tvc_angle;
    float servo_setpoint;
};
//lookup table to convert TVC angle to servo setpoint
//Grab the nearest two points and interpolate between them
// The straight bar was accidentally offset by and extra -10 degrees!
// Everything should be shifted when that's fixed
TVCToServo lookup_table_straight_bar[] = {
    // clang-format off
    {  -7.251,  -40000.0}, // -7.251 - -6.528 = -0.723
    {  -6.528,  -35000.0}, // -6.528 - -5.75 = -0.778
    {  -5.75,   -30000.0}, // -5.75 - -4.936 = -0.814
    {  -4.936,  -25000.0}, // -4.936 - -4.021 = -0.915
    {  -4.021,  -20000.0}, // -4.021 - -3.167 = -0.854
    {  -3.167,  -15000.0}, // -3.167 - -2.073 = -1.094
    {  -2.297,  -10000.0}, // -2.297 - -1.182 = -1.115
    {  -1.182,   -5000.0}, // -1.182  - 0 = -1.182
    {  0,         0000.0}, // 0 - 1.0383 = -1.0383
    {  1.0383,    5000.0}, // 1.0383 - 2.073 = -1.0347
    {  2.073,    10000.0}, // should be center // 2.073 - 3.114 = -1.041
    {  3.114,    15000.0}, // 3.114 - 4.167 = -1.053
    {  4.167,    20000.0}, // 4.167 - 5.251 = -1.084
    {  5.251,    25000.0}, // 5.251 - 6.229 = -0.978
    {  6.229,    30000.0}, // 6.229 - 7.4 = -1.171
    {  7.4,      35000.0}, // 7.4 - 8.35 = -0.95
    {  8.35,     40000.0}, // 8.35 - 9.066 = -0.716
    {  9.066,    45000.0}, // 9.066 - 9.832 = -0.766
    {  9.832,    50000.0},
    // clang-format on
};

// Okay so how this works
// The straight bar slipped. It used to be -8000, now it's -18000. That was the reference for the measurements
// simply enough, change the offset and wallah!
// Double checked with math comments
TVCToServo lookup_table_bendy_bar[] = {
    // clang-format off
    {  -7.251 - 2.073,   -40000.0 - 10000}, // -7.251 - 2.073 = -9.324         // -9.32 - -8.601 = -0.723
    {  -6.528 - 2.073,   -35000.0 - 10000}, // -6.528 - 2.073 = -8.601         // -8.601 - -7.823 = -0.778
    {  -5.75  - 2.073,   -30000.0 - 10000}, // -5.75 - 2.073 = -7.823          // -7.823 - -6.999 = -0.814
    {  -4.936 - 2.073,   -25000.0 - 10000}, // -4.936 - 2.073 = -7.009         // -7.009 - -6.094 = -0.915
    {  -4.021 - 2.073,   -20000.0 - 10000}, // -4.021 - 2.073 = -6.094         // -6.094 - -5.24 = -0.854
    {  -3.167 - 2.073,   -15000.0 - 10000}, // -3.167 - 2.073 = -5.24          // -5.24 - -4.363 = -0.877
    {  -2.297 - 2.073,   -10000.0 - 10000}, // -2.29 - 2.073 = -4.363          // -4.363 - -3.255 = -1.108
    {  -1.182 - 2.073,    -5000.0 - 10000}, // -1.182  - 2.073 = -3.255        // -3.255 - -2.073 = -1.182
    {  0      - 2.073,     0000.0 - 10000}, // 0 - 2.073 = -2.073              // -2.073 - -1.0347 = -1.0383
    {  1.0383 - 2.073,     5000.0 - 10000}, // 1.0383 - 2.073 = -1.0347
    {  2.073  - 2.073,    10000.0 - 10000},// Make this center // 2.073 - 2.073 = 0
    {  3.114  - 2.073,    15000.0 - 10000}, // 3.114 - 2.073 = 1.041
    {  4.167  - 2.073,    20000.0 - 10000}, // 4.167 - 2.073 = 2.094
    {  5.251  - 2.073,    25000.0 - 10000}, // 5.251 - 2.073 = 3.178
    {  6.229  - 2.073,    30000.0 - 10000}, // 6.229 - 2.073 = 4.156
    {  7.4    - 2.073,    35000.0 - 10000}, // 7.4 - 2.073 = 5.327
    {  8.35   - 2.073,    40000.0 - 10000}, // 8.35 - 2.073 = 6.277
    {  9.066  - 2.073,    45000.0 - 10000}, // 9.066 - 2.073 = 7.993
    {  9.832  - 2.073,    50000.0 - 10000}, // 9.832 - 2.073 = 7.759
    // clang-format on
};

float TVCAngleToServoSetpoint::convertAngleToSetpoint(float tvc_angle_degrees, bool is_straight_bar)
{
    // The TVC mount only has a range of +- 5 degrees
    if (tvc_angle_degrees > 5.0f) {
        tvc_angle_degrees = 5.0f;
    } else if (tvc_angle_degrees < -5.0f) {
        tvc_angle_degrees = -5.0f;
    }
    //printf("tvc_angle_degrees = %f\n", tvc_angle_degrees);

    // Grab the nearest two points and interpolate between them
    // The TVC mount only has a range of +- 5 degrees
    if (is_straight_bar) {
        // Find the two points that the tvc angle is between
        for (unsigned int i = 0; i < sizeof(lookup_table_straight_bar) / sizeof(lookup_table_straight_bar[0]); i++) {
            if (tvc_angle_degrees <= lookup_table_straight_bar[i].tvc_angle) {
                if (i == 0) {
                    return lookup_table_straight_bar[i].servo_setpoint;
                }
                // Interpolate between the two points
                // point 1
                auto tvc_angle_1 = lookup_table_straight_bar[i - 1].tvc_angle;
                auto servo_setpoint_1 = lookup_table_straight_bar[i - 1].servo_setpoint;
                // point 2
                auto tvc_angle_2 = lookup_table_straight_bar[i].tvc_angle;
                auto servo_setpoint_2 = lookup_table_straight_bar[i].servo_setpoint;

                //printf("STRAIGHT servo_setpoint_1 = %f, servo_setpoint_2 = %f, tvc_angle_1 = %f, tvc_angle_2 = %f, tvc_angle_degrees = %f\n",
                //       servo_setpoint_1, servo_setpoint_2, tvc_angle_1, tvc_angle_2, tvc_angle_degrees);
                //       printf("result = %f\n", servo_setpoint_1 + (servo_setpoint_2 - servo_setpoint_1) * (tvc_angle_degrees - tvc_angle_1) / (tvc_angle_2 - tvc_angle_1));
                // use std numeric lerp to interpolate
                return servo_setpoint_1 + (servo_setpoint_2 - servo_setpoint_1) * (tvc_angle_degrees - tvc_angle_1) / (tvc_angle_2 - tvc_angle_1);
            }
        }
    } else {
        for (unsigned int i = 0; i < sizeof(lookup_table_bendy_bar) / sizeof(lookup_table_bendy_bar[0]); i++) {
            if (tvc_angle_degrees <= lookup_table_bendy_bar[i].tvc_angle) {
                if (i == 0) {
                    return lookup_table_bendy_bar[i].servo_setpoint;
                }
                // Interpolate between the two points
                // point 1
                auto tvc_angle_1 = lookup_table_bendy_bar[i - 1].tvc_angle;
                auto servo_setpoint_1 = lookup_table_bendy_bar[i - 1].servo_setpoint;
                // point 2
                auto tvc_angle_2 = lookup_table_bendy_bar[i].tvc_angle;
                auto servo_setpoint_2 = lookup_table_bendy_bar[i].servo_setpoint;

                // use std numeric lerp to interpolate
                auto result = servo_setpoint_1 + (servo_setpoint_2 - servo_setpoint_1) * (tvc_angle_degrees - tvc_angle_1) / (tvc_angle_2 - tvc_angle_1);

                //printf("BENDY servo_setpoint_1 = %f, servo_setpoint_2 = %f, tvc_angle_1 = %f, tvc_angle_2 = %f, tvc_angle_degrees = %f, result = %f\n",
                //       servo_setpoint_1, servo_setpoint_2, tvc_angle_1, tvc_angle_2, tvc_angle_degrees, result);

                // The bendy bar is upside down compared to the straight bar so negate the servo setpoint
                return -1 * result;
            }
        }
    }
    return 0;
}