#ifndef PID_H
#define PID_H

// Probably just use floats but make it a template so it's easy to change
template <typename T>
class PID
{
public:
    PID(T kp, T ki, T kd, T ki_clamp);
    ~PID();

    // This takes an error from the setpoint (probably 0) on an axis of the body frame
    // TODO: degrees or radians?
    T update(T axis_angle_error_degrees, T dt);
    T getDesiredTorque_mN() const;
    void updateSetpoint(T setpoint);

private:
    //The gains
    T _kp;
    T _ki;
    T _kd;

    T _ki_clamp;

    // the setpoint
    T _setpoint;

    // The internal state
    T _previous_error;
    T _integral;

    // The output
    T _desired_torque_N;
};

template <typename T>
PID<T>::PID(T kp, T ki, T kd, T ki_clamp) : _kp(kp)
                              , _ki(ki)
                              , _kd(kd)
                              , _ki_clamp(ki_clamp)
                              , _setpoint(0)
                              , _previous_error(0)
                              , _integral(0)
{
}

template <typename T>
PID<T>::~PID()
{
}

template <typename T>
T PID<T>::update(T axis_angle_error_degrees, T dt)
{
    T error = _setpoint - axis_angle_error_degrees;
    _integral += error * dt;
    // Prevent integral windup
    if (_integral > _ki_clamp) {
        _integral = _ki_clamp;
    } else if (_integral < -_ki_clamp) {
        _integral = -_ki_clamp;
    }
    T derivative = (error - _previous_error)/dt;

    _desired_torque_N = (_kp * error) + (_ki * _integral) + (_kd * derivative);

    _previous_error = error;

    return _desired_torque_N;
}

template <typename T>
T PID<T>::getDesiredTorque_mN() const
{
    return _desired_torque_N;
}

template <typename T>
void PID<T>::updateSetpoint(T setpoint)
{
    _setpoint = setpoint;
}

#endif // PID_H
