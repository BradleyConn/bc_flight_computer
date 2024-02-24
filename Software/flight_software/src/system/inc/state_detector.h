#ifndef STATE_DETECTOR_H
#define STATE_DETECTOR_H

#include "drv_bmi088.h"
#include "drv_bmp280.h"
#include <stdbool.h>
#include <stdint.h>

class StateDetector
{
public:
    StateDetector(int32_t liftoff_acceleration_threshold_mg, int32_t burnout_acceleration_threshold_mg, int32_t apogee_altitude_threshold_from_max_cm,
                  int32_t landing_acceleration_threshold_mg);
    ~StateDetector();

    // check using bmi088 data
    bool check_liftoff_detected(const bmi088DatasetConverted& data);
    bool check_burnout_detected(const bmi088DatasetConverted& data);
    // Unlike the other checks, this one requires successive calls to check for apogee
    // The conditions are hard coded.
    bool check_apogee_detected(const int32_t altitude_cm);
    bool check_landing_detected(const bmi088DatasetConverted& data);
    // Checks to see if pitch or yaw are greater than 60 degrees
    //bool check_abort();

    // Getters using a struct
    struct StateFlags {
        bool liftoff_detected;
        bool burnout_detected;
        bool apogee_detected;
        bool landing_detected;
        bool abort;
    };
    StateFlags get_state_flags() const
    {
        return {_liftoff_detected, _burnout_detected, _apogee_detected, _landing_detected, _abort};
    }


private:
    int32_t _liftoff_acceleration_threshold_mg = 0;
    int32_t _burnout_acceleration_threshold_mg = 0;
    int32_t _apogee_altitude_threshold_from_max_cm = 0;
    int32_t _landing_acceleration_threshold_mg = 0;

    // The flags are one shot detections
    bool _liftoff_detected = false;
    bool _burnout_detected = false;
    bool _apogee_detected = false;
    bool _landing_detected = false;
    bool _abort = false;

#if 0
    // Apogee detection array to store the data history
    // Consider making this a deque if this gets big
    constexpr NUM_READINGS_PER_AVERAGE = 4;
    int32_t _apogee_detection_altitude_cm_array[NUM_READINGS_PER_AVERAGE] = {0};
    constexpr NUM_AVERAGES = 4;
    int32_t _averages[NUM_AVERAGES] = {0};
#endif
    // For apogee detection just get a max value and see if the curent value is some threshold less than that
    int32_t _max_altitude_cm = 0;
};

#endif // STATE_DETECTOR_H
