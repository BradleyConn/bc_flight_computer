#include "control_loop.h"

ControlLoop::ControlLoop(const OrientationCalculator& orientation, drv::servo& servo_yaw_Z_axis_servo_A, drv::servo& servo_pitch_Y_axis_servo_C,
                         uint64_t updateRate_us)
    : _orientation(orientation)
    , _servo_yaw_Z_axis_servo_A(servo_yaw_Z_axis_servo_A)
    , _servo_pitch_Y_axis_servo_C(servo_pitch_Y_axis_servo_C)
    , _updateRate_us(updateRate_us)
    , _time_since_update()
    , _time_under_thrust()
    , _yaw_pid(_yaw_pid_kp, _yaw_pid_ki, _yaw_pid_kd)
    , _pitch_pid(_pitch_pid_kp, _pitch_pid_ki, _pitch_pid_kd)
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
}

void ControlLoop::tryUpdate(const bmi088DatasetConverted& data)
{
    if (_time_since_update.deltaTime_us() < _updateRate_us || _abort) {
        return;
    }
    float dt_s_f = _time_since_update.deltaTime_us() / 1000000.0f;

    // 1. Get the rocket body frame yaw and pitch errors
    auto _angle_errors = convertEulerAnglesToAxisError(_orientation.getEulerYawPitchRollAngles());

    // 2. Feed the errors into the PID controllers
    auto yaw_torque_mN = _yaw_pid.update(_angle_errors.yaw_Z_axis_servo_A_error_degrees, dt_s_f);
    auto pitch_torque_mN = _pitch_pid.update(_angle_errors.pitch_y_axis_servo_C_error_degrees, dt_s_f);

    // 3. Convert the torques to TVC angles
    auto _time_under_thrust_ms = _time_under_thrust.deltaTime_ms();
    auto yaw_tvc_angle = _torque_to_tvc_angle.getTvcAngle(yaw_torque_mN, _time_under_thrust_ms);
    auto pitch_tvc_angle = _torque_to_tvc_angle.getTvcAngle(pitch_torque_mN, _time_under_thrust_ms);

    // 4. Convert the TVC angles to servo setpoints as the servos are acting as linear actuators. It's not direct drive rotation.
    auto yaw_servo_setpoint = _tvc_angle_to_servo_setpoint.convertAngleToSetpoint(yaw_tvc_angle);
    auto pitch_servo_setpoint = _tvc_angle_to_servo_setpoint.convertAngleToSetpoint(pitch_tvc_angle);

    // 5. Convert the angle floats to milliangle integers
    auto yaw_servo_setpoint_millidegrees = static_cast<int64_t>(yaw_servo_setpoint * 1000);
    auto pitch_servo_setpoint_millidegrees = static_cast<int64_t>(pitch_servo_setpoint * 1000);

    // 6. Move the servos
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(yaw_servo_setpoint_millidegrees);
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(pitch_servo_setpoint_millidegrees);

    // 7. Update the time since the last update
    _time_since_update.mark();
}

void ControlLoop::abort()
{
    _abort = true;
    _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(0);
    _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(0);
}
