#include "../../inc/controls/euler_angle_error_calculator.h"
#include <cmath>

// TODO: Make a util file for these
#define RAD_TO_DEG 57.295779513082320876798154814105

// NOTE: This is all in radians
AxisAngleErrors convertEulerAnglesToAxisError(const EulerAngles& euler)
{
    auto yaw = euler.yaw;
    auto pitch = euler.pitch;
    auto roll = euler.roll;

    // Okay this implementation is going to be a little bit unusual. This is what's going on.
    // The angles are provided in the Yaw Pitch Roll euler angle format.
    // That means Yaw THEN Pitch THEN Roll.
    // But we want it so be in Yaw and Pitch SIMULTANEOUSLY. No roll control here.
    // So the way I think about it the Yaw and Pitch should be a single vector.
    // This makes sense if you think of a Roll, Pitch, Roll euler angle representation.
    // The pitch is that single vector.
    // The second Roll determines how much of that vector is aligned with the body frame yaw and pitch axes.
    // It is also important to split this up into the component parts before the PID controller otherwise the I term doesn't look at the servos individually.
    // And this is important because the TVC mount has some misalignment which the I term should correct for.
    // So the plan is to try and get to the Roll, Pitch, Roll euler angle representation.
    // This certainly could come straight from the quaternion representation but that's not where I'm going with this.
    // 1. The plan is to find the pitch error first, which I internally reference as the magnitude of the angle error.
    // 2. Then I want to find the roll so the vector can be split into the component parts. I refer to this as rotation.
    // 3. Then take these values and calculate the component parts and assign them to the rocket body frame axes.

    // 1.
    // It took some and rotating objects in my head to land here, but a great way to think about euler angles is to think about a globe.
    // The yaw is the longitude and the pitch is the latitude.
    // The roll is the rotation around the axis that goes through the center of the globe.
    // The key thing to think about is that the yaw and pitch are always facing the North Pole.
    // This is an important realization.

    // To calculate the magnitude of the angle error it's simply the following formula
    // mag = arccos(cos(yaw) * cos(pitch))
    // I used great circle distance formula found at the link below to derive this
    // https://en.wikipedia.org/wiki/Great-circle_distance#:%7E:text=.%5B1%5D-,Formulae,-%5Bedit%5D
    // The formula is arccos(sin(phi1) * sin(phi2) + cos(phi1) * cos(phi2) * cos(lambda1 - lambda2))
    // where phi1 and phi2 are the latitudes and lambda1 and lambda2 are the longitudes
    // For this, we want to know the distance from the current position (yaw, pitch) to the origin (0,0)
    // so phi1 = pitch, phi2 = 0, lambda1 = yaw, lambda2 = 0
    // Substituting the values we get
    // arccos(sin(0) * sin(pitch) + cos(0) * cos(pitch) * cos(yaw - 0))
    // sin(0) = 0, so the sin terms drop out
    // cos(0) = 1
    // cos(yaw - 0) = cos(yaw)
    // so the formula becomes
    // arccos(0 + 1 * cos(pitch) * cos(yaw))
    // which becomes
    // arccos(cos(pitch) * cos(yaw))

    auto mag_radians = acos(cos(pitch) * cos(yaw));

    // 2.
    // The roll on the other hand is much more complicated
    // So you may think the roll is just the roll angle but that's only part of it.
    // Thinking about this like a globe, the yaw is the longitude and the pitch is the latitude.
    // In Yaw Pitch Roll euler angles, before the roll is taken into account, the Yaw and Pitch are always facing the North Pole.
    // that means even if the roll is always zero, there's some roll that needs to be applied to get the rocket to face the (0,0) coordinate on the globe.
    // The way I think about it, there's an "induced roll" by the way the euler angles are defined,
    // as the roll needed to be facing the (0,0) coordinate on the globe is not fixed, it actually varies.

    // Let's calculate that roll.
    // Instead of explaing it here you can follow this
    // https://math.stackexchange.com/questions/4853529/given-latitude-and-longitude-and-facing-north-how-can-i-calculate-the-rotation

    // Again using the globe analogy:
    // yaw = longitude = phi
    // pitch = latitude = theta
    auto cos_phi = cos(yaw);
    auto sin_phi = sin(yaw);
    auto cos_theta = cos(pitch);
    auto sin_theta = sin(pitch);

    auto w_x = 0.0;
    auto w_y = sin_theta;
    auto w_z = -cos_theta * sin_phi;

    auto n_x = -sin_theta * cos_phi;
    auto n_y = -sin_theta * sin_phi;
    auto n_z = cos_theta;

    auto e_x = -sin_phi;
    auto e_y = cos_phi;
    auto e_z = 0.0;

    auto w_dot_n = w_x * n_x + w_y * n_y + w_z * n_z;
    auto w_dot_e = w_x * e_x + w_y * e_y + w_z * e_z;

    auto rotation_offset = atan2(-w_dot_n, w_dot_e);

    // And continuing to follow that it needs to be determined if it needs a PI radians (180 degree) rotation
    auto p_x = cos_theta * cos_phi;
    auto p_y = cos_theta * sin_phi;
    auto p_z = sin_theta;
    auto q_minus_p_x = 1.0 - p_x;
    auto q_minus_p_y = 0.0 - p_y;
    auto q_minus_p_z = 0.0 - p_z;

    auto sin_rotate = sin(rotation_offset);
    auto cos_rotate = cos(rotation_offset);
    auto cos_rotate_n_x = cos_rotate * n_x;
    auto cos_rotate_n_y = cos_rotate * n_y;
    auto cos_rotate_n_z = cos_rotate * n_z;
    auto sin_rotate_e_x = sin_rotate * e_x;
    auto sin_rotate_e_y = sin_rotate * e_y;
    auto sin_rotate_e_z = sin_rotate * e_z;
    auto v_x = cos_rotate_n_x + sin_rotate_e_x;
    auto v_y = cos_rotate_n_y + sin_rotate_e_y;
    auto v_z = cos_rotate_n_z + sin_rotate_e_z;

    auto dot_product = v_x * q_minus_p_x + v_y * q_minus_p_y + v_z * q_minus_p_z;

    // If it's negative need to change the rotation offset by PI radians (180 degrees)
    if (dot_product < 0.0) {
        // Keep it less than abs(180 deg), so check if it needs addition or subtraction
        if (rotation_offset > 0.0) {
            rotation_offset -= M_PI;
        } else {
            rotation_offset += M_PI;
        }
    }

    // So now the "induced rotation" is known
    // Add the roll angle to it and that's the total roll needed to face the (0,0) coordinate on the globe
    auto full_rotation_offset_radians = rotation_offset - roll;

    // To determine the extra roll in Roll, Pitch, Roll euler angles you could do the following:
#if 0
    // Honestly this should be it's own helper function...
    // First pull the orientation library in
    // Would be better to grab a copy of the quaternions instead of using the euler angles to calculate the quaternions but
    // get the quaternion representation of the euler angles
    auto orientation_copy = Orientation();
    orientation_copy.update(euler.yaw, 0.0, 0.0, 1.0);
    orientation_copy.update(0.0, euler.pitch, 0.0, 1.0);
    orientation_copy.update(0.0, 0.0, euler.roll, 1.0);

    // For clarity
    auto pitch_radians = mag_radians;
    auto roll_radians = full_rotation_offset_radians;

    // Now reverse out the roll and pitch to get back to the origin and see what the last roll offset at the origin is
    orientation_copy.update(0.0, 0.0, rp_result.roll_radians, 1.0);
    orientation_copy.update(0.0, rp_result.pitch_radians, 0.0, 1.0);
    auto roll_at_origin = orientation_copy.toEuler().roll;
    // Not sure where things should be negated or reversed but this is enough.
#endif

    // 3.
    // Now split it up into the component parts
    auto yaw_contribution_rad = mag_radians * sin(full_rotation_offset_radians);
    auto pitch_contribution_rad = mag_radians * cos(full_rotation_offset_radians);

    AxisAngleErrors result;
    result.yaw_Z_axis_servo_A_error_degrees = yaw_contribution_rad * RAD_TO_DEG;
    result.pitch_y_axis_servo_C_error_degrees = pitch_contribution_rad * RAD_TO_DEG;

    return result;
}