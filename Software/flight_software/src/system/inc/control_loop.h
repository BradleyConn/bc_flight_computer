#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

#include "controls/PID.h"
#include "controls/euler_angle_error_calculator.h"
#include "controls/torque_to_tvc_angle.h"
#include "controls/tvc_angle_to_servo_setpoint.h"
#include "drv_bmi088.h"
#include "drv_servo.h"
#include "orientation_calculator.h"
#include "time_keeper.h"
#include <stdint.h>

class ControlLoop
{
public:
    //injection of the servos
    ControlLoop(const OrientationCalculator& orientation, drv::servo& servo_yaw_Z_axis_servo_A, drv::servo& servo_pitch_Y_axis_servo_C, uint64_t updateRate_us);
    ~ControlLoop();

    void start();
    // Run control loop if it has been long enough since the last update
    void tryUpdate(const bmi088DatasetConverted& data);
    void abort();

private:
    const OrientationCalculator& _orientation;
    // The two servos
    drv::servo& _servo_yaw_Z_axis_servo_A;
    drv::servo& _servo_pitch_Y_axis_servo_C;
    uint64_t _updateRate_us;

    TimeKeeper _time_since_update;
    TimeKeeper _time_under_thrust;

    // The PID controllers
    PID<float> _yaw_pid;
    PID<float> _pitch_pid;
    TorqueToTvcAngle _torque_to_tvc_angle;
    TVCAngleToServoSetpoint _tvc_angle_to_servo_setpoint;

    // The PID values! Proabably better as parameters
    static constexpr float _yaw_pid_kp = 1.0f;
    static constexpr float _yaw_pid_ki = 0.0f;
    static constexpr float _yaw_pid_kd = 0.0f;

    static constexpr float _pitch_pid_kp = 1.0f;
    static constexpr float _pitch_pid_ki = 0.0f;
    static constexpr float _pitch_pid_kd = 0.0f;

    bool _abort = false;
};

#endif // CONTROL_LOOP_H
