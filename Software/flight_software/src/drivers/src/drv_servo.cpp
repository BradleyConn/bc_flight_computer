#include "../inc/drv_servo.h"
#include "hardware/pwm.h"

namespace drv {
servo::servo(uint gpio, servo_type type, int32_t starting_angle_centi_degrees)
    : _gpio(gpio)
    , _type(type)
{
    //125mhz gives 8ns period
    //20ms/8ns = 2,500,000 = "wrap" (minus 1 because 0 indexed)
    //then 1.5ms/8ns = 187,500 = "level" (minus 1 because 0 indexed)

    gpio_set_function(_gpio, GPIO_FUNC_PWM);

    _slice_num = pwm_gpio_to_slice_num(_gpio);
    _channel = pwm_gpio_to_channel(_gpio);

    // servo takes a uint16_t which is 65535. Can't fit 2,500,000
    // 2,500,000/65535 ~=38. Use 64 div
//    if (_gpio == 22) {
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, 64);
    pwm_init(_slice_num, &config, true);
//    }

    // The max counter
    if (_type == servo_type::Analog) {
        pwm_set_wrap(_slice_num, 2500000/64);
    }
    else {
        // Analog is 50hz, digital can go up to 333hz. Some internet peeps seem to think is can overheat if you go that fast.
        // 4x shorter period means 4x faster, so 200hz. Should be a good start
        pwm_set_wrap(_slice_num, 2500000/4/64);
    }
    // The threshold PWM is ON. Put it in the middle.
    set_angle_centi_degrees(starting_angle_centi_degrees);
    // Set the PWM running
    pwm_set_enabled(_slice_num, true);
}

servo::~servo()
{
    pwm_set_enabled(_slice_num, false);
}

void servo::set_angle_centi_degrees(int32_t centi_degrees)
{
    // TODO: Maybe error check the input?
    // -90 is 125,000
    //   0 is 187,500
    // +90 is 250,000
    
    // One step calculations
    // 125,000 total servo range
    // Degree input ranges from -9,000 to +9,000 (-90*100, +90*100)
    // 18,000 total range
    // 125,000/18,000 = 6.94444 per centi degree
    // or 694 per degree
    constexpr auto step_per_degree = 125000/180;
    constexpr auto midpoint_offset = 187500;

    // For better resolution use centi-degree then div by 100
    auto centi_degree_steps = step_per_degree * centi_degrees;
    auto angle_steps = midpoint_offset + (centi_degree_steps/100);
    pwm_set_chan_level(_slice_num, _channel, angle_steps/64);

}

} //namespace drv
