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
    ControlLoop(const OrientationCalculator& orientation, drv::servo& servo_yaw_Z_axis_servo_A, drv::servo& servo_pitch_Y_axis_servo_C, uint64_t updateRate_us,
                float lever_arm_m, float mmoi_kg_m2);
    ~ControlLoop();

    void start();
    // Run control loop if it has been long enough since the last update
    void tryUpdate(const bmi088DatasetConverted& data);
    void abort();
    // This check is done by visual inspection of the rocket
    void init_servo_test();

    struct ControlLoopData {
        float yaw_error_degrees;
        float pitch_error_degrees;
        float yaw_PID_angle;
        float pitch_PID_angle;
        float yaw_tvc_angle;
        float pitch_tvc_angle;
        float yaw_servo_setpoint;
        float pitch_servo_setpoint;
        // time since last update
        uint32_t time_since_update_us;
        // time under thrust
        uint32_t time_under_thrust_ms;
    };

    ControlLoopData getControlLoopData() const
    {
        return _control_loop_data;
    }

    //hack to see if the loop updated
    bool did_update = false;

private:
    const OrientationCalculator& _orientation;
    // The two servos
    drv::servo& _servo_yaw_Z_axis_servo_A;
    drv::servo& _servo_pitch_Y_axis_servo_C;
    uint64_t _updateRate_us;
    float _lever_arm_m;
    float _mmoi_kg_m2;

    TimeKeeper _time_since_update;
    TimeKeeper _time_under_thrust;

    // The PID controllers
    PID<float> _yaw_pid;
    PID<float> _pitch_pid;
    TorqueToTvcAngle _torque_to_tvc_angle;
    TVCAngleToServoSetpoint _tvc_angle_to_servo_setpoint;

    // The PID values! Proabably better as parameters
    static constexpr float _yaw_pid_kp = 0.4f;
    static constexpr float _yaw_pid_ki = 0.02f;
    static constexpr float _yaw_pid_kd = 0.2f;

    static constexpr float _pitch_pid_kp = 0.4f;
    static constexpr float _pitch_pid_ki = 0.02f;
    static constexpr float _pitch_pid_kd = 0.2f;

    static constexpr float _ki_clamp = 5 * 4.5f; // Roughly tvc angle (5 deg) * servo linkage ratio (4.5:1)

    bool _abort = false;

    // controll loop data to hold the state
    ControlLoopData _control_loop_data;
};

#endif // CONTROL_LOOP_H
