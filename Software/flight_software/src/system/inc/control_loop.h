#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

#include "controls/PID.h"
#include "controls/euler_angle_error_calculator.h"
#include "controls/quaternion_orientation.h"
#include "controls/torque_to_tvc_angle.h"
#include "controls/tvc_angle_to_servo_setpoint.h"
#include "drv_bmi088.h"
#include "drv_servo.h"
#include "time_keeper.h"
#include <stdint.h>

class ControlLoop
{
public:
//injection of the servos
    ControlLoop(drv::servo &servo_yaw_Z_axis_servo_A, drv::servo &servo_pitch_Y_axis_servo_C);
    ~ControlLoop();

    // update takes the bmi088 data and the dt in seconds and moves the servos
    void update(bool liftoff_detected, const bmi088DatasetConverted &data, float dt);

private:
    // The orientation
    QuaternionOrientation _orientation;
    // The angle error calculator output
    AxisAngleErrors _angle_errors;
    // The PID controllers
    PID<float> _yaw_pid;
    PID<float> _pitch_pid;
    TorqueToTvcAngle _torque_to_tvc_angle;
    TVCAngleToServoSetpoint _tvc_angle_to_servo_setpoint;

    // The two servos
    drv::servo & _servo_yaw_Z_axis_servo_A;
    drv::servo & _servo_pitch_Y_axis_servo_C;

    bool _liftoff_detected = false;
    // Need to keep track of when liftoff was detected
    TimeKeeper _liftoff_time_keeper;


    // The PID values! Proabably better as parameters
    static constexpr float _yaw_pid_kp = 1.0f;
    static constexpr float _yaw_pid_ki = 0.0f;
    static constexpr float _yaw_pid_kd = 0.0f;

    static constexpr float _pitch_pid_kp = 1.0f;
    static constexpr float _pitch_pid_ki = 0.0f;
    static constexpr float _pitch_pid_kd = 0.0f;
};

#endif // CONTROL_LOOP_H
