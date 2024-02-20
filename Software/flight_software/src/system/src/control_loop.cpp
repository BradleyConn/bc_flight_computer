#include "control_loop.h"

ControlLoop::ControlLoop(drv::servo& servo_yaw_Z_axis_servo_A, drv::servo& servo_pitch_Y_axis_servo_C)
    : _yaw_pid(_yaw_pid_kp, _yaw_pid_ki, _yaw_pid_kd)
    , _pitch_pid(_pitch_pid_kp, _pitch_pid_ki, _pitch_pid_kd)
    , _servo_yaw_Z_axis_servo_A(servo_yaw_Z_axis_servo_A)
    , _servo_pitch_Y_axis_servo_C(servo_pitch_Y_axis_servo_C)
    , _liftoff_time_keeper()
{
}

ControlLoop::~ControlLoop()
{
}

void ControlLoop::update(bool liftoff_detected, const bmi088DatasetConverted& data, float dt)
{
    // If this is the first time liftoff has been detected, mark the time
    if (! _liftoff_detected && liftoff_detected) {
        _liftoff_time_keeper.mark();
        _liftoff_detected = true;
    }

    if (_liftoff_detected) {
        // Get the time right away as these calls will take time
        auto time_since_liftoff_ms = _liftoff_time_keeper.deltaTime_ms();

        // 1. Update the orientation
        _orientation.update(data, dt);
        // 2. Get the rocket body frame yaw and pitch errors
        _angle_errors = convertEulerAnglesToAxisError(_orientation.getEulerYawPitchRollAngles());

        // 3. Feed the errors into the PID controllers
        auto yaw_torque_mN = _yaw_pid.update(_angle_errors.yaw_Z_axis_servo_A_error_degrees, dt);
        auto pitch_torque_mN = _pitch_pid.update(_angle_errors.pitch_y_axis_servo_C_error_degrees, dt);

        // 4. Convert the torques to TVC angles
        auto yaw_tvc_angle = _torque_to_tvc_angle.getTvcAngle(yaw_torque_mN, time_since_liftoff_ms);
        auto pitch_tvc_angle = _torque_to_tvc_angle.getTvcAngle(pitch_torque_mN, time_since_liftoff_ms);

        // 5. Convert the TVC angles to servo setpoints as the servos are acting as linear actuators. It's not direct drive rotation.
        auto yaw_servo_setpoint = _tvc_angle_to_servo_setpoint.convertAngleToSetpoint(yaw_tvc_angle);
        auto pitch_servo_setpoint = _tvc_angle_to_servo_setpoint.convertAngleToSetpoint(pitch_tvc_angle);

        // 6. Convert the angle floats to milliangle integers
        auto yaw_servo_setpoint_millidegrees = static_cast<int64_t>(yaw_servo_setpoint * 1000);
        auto pitch_servo_setpoint_millidegrees = static_cast<int64_t>(pitch_servo_setpoint * 1000);

        // 7. Move the servos
        _servo_yaw_Z_axis_servo_A.set_angle_milli_degrees(yaw_servo_setpoint_millidegrees);
        _servo_pitch_Y_axis_servo_C.set_angle_milli_degrees(pitch_servo_setpoint_millidegrees);
    } else {
        // liftoff not detected, just keep updating the orientation with the gravity vector
        _orientation.update_gravity(data, dt);
    }
}
