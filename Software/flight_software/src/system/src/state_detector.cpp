#include "state_detector.h"
#include <math.h>
#include <stdio.h>

StateDetector::StateDetector(int32_t liftoff_acceleration_threshold_mg, int32_t burnout_acceleration_threshold_mg,
                             int32_t apogee_altitude_threshold_from_max_cm, int32_t landing_acceleration_threshold_mg)
    : _liftoff_acceleration_threshold_mg(liftoff_acceleration_threshold_mg)
    , _burnout_acceleration_threshold_mg(burnout_acceleration_threshold_mg)
    , _apogee_altitude_threshold_from_max_cm(apogee_altitude_threshold_from_max_cm)
    , _landing_acceleration_threshold_mg(landing_acceleration_threshold_mg)
{
}

StateDetector::~StateDetector()
{
}
// check using bmi088 data
bool StateDetector::check_liftoff_detected(const bmi088DatasetConverted& data)
{
    if (_liftoff_detected) {
        return true;
    }
    // check the magnitude of the acceleration
    // TODO: Time this to see if it is too slow
    float magnitude = sqrt(data.accel_data_converted.x_mg * data.accel_data_converted.x_mg + data.accel_data_converted.y_mg * data.accel_data_converted.y_mg +
                           data.accel_data_converted.z_mg * data.accel_data_converted.z_mg);
    if (magnitude > (float)_liftoff_acceleration_threshold_mg) {
        _liftoff_detected = true;
        //printf("Liftoff detected! Magnitude: %f, threshold: %f\n", magnitude, (float)_liftoff_acceleration_threshold_mg);
        return true;
    }
    return false;
}

bool StateDetector::check_burnout_detected(const bmi088DatasetConverted& data)
{
    if (_burnout_detected) {
        return true;
    }
    // check if the magnitude of the acceleration is less than the burnout threshold
    // TODO: Time this to see if it is too slow
    float magnitude = sqrt(data.accel_data_converted.x_mg * data.accel_data_converted.x_mg + data.accel_data_converted.y_mg * data.accel_data_converted.y_mg +
                           data.accel_data_converted.z_mg * data.accel_data_converted.z_mg);
    if (magnitude < (float)_burnout_acceleration_threshold_mg) {
        _burnout_detected = true;
        //printf("Burnout detected! Magnitude: %f, threshold: %f\n", magnitude, (float)_burnout_acceleration_threshold_mg);
        return true;
    }
    return false;
}

// Check using bmp280 data
bool StateDetector::check_apogee_detected(const int32_t altitude_cm)
{
#if 0
    if (_apogee_detected) {
        return true;
    }
    // Shift the readings array to the left
    for (int i = 0; i < NUM_READINGS_PER_AVERAGE - 1; i++) {
        _apogee_detection_altitude_cm_array[i] = _apogee_detection_altitude_cm_array[i + 1];
    }
    // Add the new reading to the end
    _apogee_detection_altitude_cm_array[NUM_READINGS_PER_AVERAGE - 1] = altitude_cm;

    // Shift the averages array to the left
    for (int i = 0; i < NUM_AVERAGES - 1; i++) {
        _averages[i] = _averages[i + 1];
    }

    // Calculate the average of the last num_readings_per_average readings
    int64_t sum = 0;
    for (int i = 0; i < NUM_READINGS_PER_AVERAGE; i++) {
        sum += _apogee_detection_altitude_cm_array[i];
    }
    _averages[NUM_AVERAGES - 1] = sum / NUM_READINGS_PER_AVERAGE;

    // Check if the last averages are
#endif

    if (altitude_cm > _max_altitude_cm) {
        _max_altitude_cm = altitude_cm;
    }
    if (_max_altitude_cm - altitude_cm > _apogee_altitude_threshold_from_max_cm) {
        _apogee_detected = true;
        return true;
    }

    return false;
}

bool StateDetector::check_landing_detected(const bmi088DatasetConverted& data)
{
    if (_landing_detected) {
        return true;
    }
    // check if the magnitude of the acceleration is less than the landing threshold
    float magnitude = sqrt(data.accel_data_converted.x_mg * data.accel_data_converted.x_mg + data.accel_data_converted.y_mg * data.accel_data_converted.y_mg +
                           data.accel_data_converted.z_mg * data.accel_data_converted.z_mg);
    if (magnitude < (float)_landing_acceleration_threshold_mg) {
        _landing_detected = true;
        //printf("Landing detected! Magnitude: %f, threshold: %f\n", magnitude, (float)_landing_acceleration_threshold_mg);
        return true;
    }
    return false;
}

//ool StateDetector::check_abort(const bmi088DatasetConverted& data)
//{
// TODO: implement this
//    return false;
//}