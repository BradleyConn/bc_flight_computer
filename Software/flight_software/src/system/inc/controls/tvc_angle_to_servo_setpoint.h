#ifndef TVC_ANGLE_TO_SERVO_SETPOINT_H
#define TVC_ANGLE_TO_SERVO_SETPOINT_H

// This is needed because the servo acts a linear actuator, not a 1 to 1 rotation
class TVCAngleToServoSetpoint {
public:
    TVCAngleToServoSetpoint();
    ~TVCAngleToServoSetpoint();

    float convertAngleToSetpoint(float tvc_angle_degrees, bool is_straight_bar);
};

#endif // TVC_ANGLE_TO_SERVO_SETPOINT_H
