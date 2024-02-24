#include "control_loop.h"
#include <stdio.h>

ControlLoop::ControlLoop(const OrientationCalculator& orientation, drv::servo& servo_yaw_Z_axis_servo_A, drv::servo& servo_pitch_Y_axis_servo_C,
                         uint64_t updateRate_us, float lever_arm_m, float mmoi_kg_m2)
    : _orientation(orientation)
    , _servo_yaw_Z_axis_servo_A(servo_yaw_Z_axis_servo_A)
    , _servo_pitch_Y_axis_servo_C(servo_pitch_Y_axis_servo_C)
    , _updateRate_us(updateRate_us)
    , _lever_arm_m(lever_arm_m)
    , _mmoi_kg_m2(mmoi_kg_m2)
    , _time_since_update()
    , _time_under_thrust()
    , _yaw_pid(_yaw_pid_kp, _yaw_pid_ki, _yaw_pid_kd, _ki_clamp)
    , _pitch_pid(_pitch_pid_kp, _pitch_pid_ki, _pitch_pid_kd, _ki_clamp)
    , _torque_to_tvc_angle()
    , _tvc_angle_to_servo_setpoint()
    , _abort(false)
{
}

ControlLoop::~ControlLoop()
{
}

void ControlLoop::start()
{
    _time_under_thrust.mark();
    _time_since_update.mark();
}

void ControlLoop::tryUpdate(const bmi088DatasetConverted& data)
{
    if (_time_since_update.deltaTime_us() < _updateRate_us || _abort) {
        did_update = false;
        return;
    }
    did_update = true;
    float dt_s_f = _time_since_update.deltaTime_us() / (1000.0f * 1000.0f);

    // 1. Get the rocket body frame yaw and pitch errors
    auto _angle_errors = convertEulerAnglesToAxisError(_orientation.getEulerYawPitchRollAngles());
    //printf("yaw = %f, pitch = %f, \n", _angle_errors.yaw_Z_axis_servo_A_error_degrees, _angle_errors.pitch_y_axis_servo_C_error_degrees);

#if 0
    // Intended to use torque based PID but I'm getting some trouble. I'll use the angle based PID for now.
    // 2. Feed the errors into the PID controllers
    auto yaw_PID_torque_NM = _yaw_pid.update(_angle_errors.yaw_Z_axis_servo_A_error_degrees, dt_s_f);
    auto pitch_PID_torque_NM = _pitch_pid.update(_angle_errors.pitch_y_axis_servo_C_error_degrees, dt_s_f);

    // 3. Convert the torques to TVC angles
    auto _time_under_thrust_ms = _time_under_thrust.deltaTime_ms();
    auto yaw_tvc_angle = _torque_to_tvc_angle.getTvcAngle(yaw_PID_torque_NM, _time_under_thrust_ms, _lever_arm_m);
    auto pitch_tvc_angle = _torque_to_tvc_angle.getTvcAngle(pitch_PID_torque_NM, _time_under_thrust_ms, _lever_arm_m);

    // 4. Convert the TVC angles to servo setpoints as the servos are acting as linear actuators. It's not direct drive rotation.
    auto yaw_servo_setpoint = _tvc_angle_to_servo_setpoint.convertAngleToSetpoint(yaw_tvc_angle, /* is straight bar*/ true);
    auto pitch_servo_setpoint = _tvc_angle_to_servo_setpoint.convertAngleToSetpoint(pitch_tvc_angle, /* is straight bar*/ false);

    //printf("yaw_PID_torque_NM = %f, pitch_PID_torque_NM = %f, yaw_tvc_angle = %f, pitch_tvc_angle = %f, yaw_servo_setpoint = %f, pitch_servo_setpoint = %f\n",
    //       yaw_PID_torque_NM, pitch_PID_torque_NM, yaw_tvc_angle, pitch_tvc_angle, yaw_servo_setpoint, pitch_servo_setpoint);
#else
    // 2. Feed the errors into the PID controllers
    auto yaw_PID_angle = _yaw_pid.update(_angle_errors.yaw_Z_axis_servo_A_error_degrees, dt_s_f);
    auto pitch_PID_angle = _pitch_pid.update(_angle_errors.pitch_y_axis_servo_C_error_degrees, dt_s_f);

    // 3. Convert the angles to TVC angles
    auto yaw_tvc_angle = yaw_PID_angle;
    auto pitch_tvc_angle = pitch_PID_angle;

    // 4. Convert the TVC angles to servo setpoints as the servos are acting as linear actuators. It's not direct drive rotation.
    auto yaw_servo_setpoint = _tvc_angle_to_servo_setpoint.convertAngleToSetpoint(yaw_tvc_angle, /* is straight bar*/ true);
    auto pitch_servo_setpoint = _tvc_angle_to_servo_setpoint.convertAngleToSetpoint(pitch_tvc_angle, /* is straight bar*/ false);

    //printf("yaw_PID_angle = %f, pitch_PID_ANGLE = %f, yaw_tvc_angle = %f, pitch_tvc_angle = %f, yaw_servo_setpoint = %f, pitch_servo_setpoint = %f\n",
    //       yaw_PID_angle, pitch_PID_angle, yaw_tvc_angle, pitch_tvc_angle, yaw_servo_setpoint, pitch_servo_setpoint);
#endif

    // 6. Move the servos
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(yaw_servo_setpoint);
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(pitch_servo_setpoint);

    // 7. Update the time since the last update
    _time_since_update.mark();

    // copy all the state to the data struct
    _control_loop_data = { _angle_errors.yaw_Z_axis_servo_A_error_degrees,
                           _angle_errors.pitch_y_axis_servo_C_error_degrees,
                           yaw_PID_angle,
                           pitch_PID_angle,
                           yaw_tvc_angle,
                           pitch_tvc_angle,
                           yaw_servo_setpoint,
                           pitch_servo_setpoint,
                           (uint32_t)_time_since_update.deltaTime_us(),
                           (uint32_t)_time_under_thrust.deltaTime_ms() };

}

void ControlLoop::abort()
{
    _abort = true;
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(0);
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(0);
}

void ControlLoop::init_servo_test()
{
    auto tvc_yaw_5 = _tvc_angle_to_servo_setpoint.convertAngleToSetpoint(5, true);
    auto tvc_pitch_5 = _tvc_angle_to_servo_setpoint.convertAngleToSetpoint(5, false);
    auto tvc_yaw_neg_5 = _tvc_angle_to_servo_setpoint.convertAngleToSetpoint(-5, true);
    auto tvc_pitch_neg_5 = _tvc_angle_to_servo_setpoint.convertAngleToSetpoint(-5, false);
    const int sleep_duration_ms = 200;

    // Go west (yaw 5), North (pitch 5), East (yaw -5), South (pitch -5)
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(static_cast<int64_t>(tvc_yaw_5));
    sleep_ms(sleep_duration_ms);
    //back to 0
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(0);
    sleep_ms(sleep_duration_ms);
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(static_cast<int64_t>(tvc_pitch_5));
    sleep_ms(sleep_duration_ms);
    //back to 0
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(0);
    sleep_ms(sleep_duration_ms);
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(static_cast<int64_t>(tvc_yaw_neg_5));
    sleep_ms(sleep_duration_ms);
    //back to 0
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(0);
    sleep_ms(sleep_duration_ms);
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(static_cast<int64_t>(tvc_pitch_neg_5));
    sleep_ms(sleep_duration_ms);
    //back to 0
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(0);
    sleep_ms(sleep_duration_ms);

    // Now do a figure 8
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(static_cast<int64_t>(tvc_yaw_5));
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(static_cast<int64_t>(tvc_pitch_5));
    sleep_ms(sleep_duration_ms);
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(static_cast<int64_t>(tvc_yaw_neg_5));
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(static_cast<int64_t>(tvc_pitch_neg_5));
    sleep_ms(sleep_duration_ms);
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(static_cast<int64_t>(tvc_yaw_5));
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(static_cast<int64_t>(tvc_pitch_neg_5));
    sleep_ms(sleep_duration_ms);
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(static_cast<int64_t>(tvc_yaw_neg_5));
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(static_cast<int64_t>(tvc_pitch_5));
    sleep_ms(sleep_duration_ms);
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(static_cast<int64_t>(tvc_yaw_5));
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(static_cast<int64_t>(tvc_pitch_5));
    sleep_ms(sleep_duration_ms);
    //back to 0
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(0);
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(0);
    sleep_ms(sleep_duration_ms);

    //Now do a square
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(static_cast<int64_t>(tvc_yaw_5));
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(static_cast<int64_t>(tvc_pitch_5));
    sleep_ms(sleep_duration_ms);
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(static_cast<int64_t>(tvc_yaw_neg_5));
    sleep_ms(sleep_duration_ms);
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(static_cast<int64_t>(tvc_pitch_neg_5));
    sleep_ms(sleep_duration_ms);
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(static_cast<int64_t>(tvc_yaw_5));
    sleep_ms(sleep_duration_ms);
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(static_cast<int64_t>(tvc_pitch_5));
    sleep_ms(sleep_duration_ms);
    //back to 0
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(0);
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(0);
    sleep_ms(sleep_duration_ms);
}
