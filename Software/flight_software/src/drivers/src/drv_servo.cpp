#include "../inc/drv_servo.h"
#include "hardware/pwm.h"
#include "drv_servo.h"
#include "stdio.h"

namespace drv
{
servo::servo(uint gpio, servo_type type, int64_t offset_milli_degree, int64_t starting_angle_milli_degrees)
    : _gpio(gpio)
    , _type(type)
    , _offset_milli_degree(offset_milli_degree)
{
    //125mhz gives 8ns period
    //20ms/8ns = 2,500,000 = "wrap" (minus 1 because 0 indexed)
    //then 1.5ms/8ns = 187,500 = "level" (minus 1 because 0 indexed)

    gpio_set_function(_gpio, GPIO_FUNC_PWM);

    _slice_num = pwm_gpio_to_slice_num(_gpio);
    _channel = pwm_gpio_to_channel(_gpio);

    // servo takes a uint16_t which is 65535. Can't fit 2,500,000
    // 2,500,000/65535 ~=38. Use 64 div
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, 64);
    pwm_init(_slice_num, &config, true);

    // The max counter
    if (_type == servo_type::Analog) {
        pwm_set_wrap(_slice_num, 2500000 / 64);
    } else {
        // Analog is 50hz, digital can go up to 333hz. Some internet peeps seem to think is can overheat if you go that fast.
        // 4x shorter period means 4x faster, so 200hz. Should be a good start
        pwm_set_wrap(_slice_num, (2500000 / 4) / 64);
    }
    // The threshold PWM is ON. Put it in the middle.
    set_angle_milli_degrees(starting_angle_milli_degrees);
    // Set the PWM running
    pwm_set_enabled(_slice_num, true);
}

servo::~servo()
{
    pwm_set_enabled(_slice_num, false);
}

void servo::set_angle_centi_degrees(int64_t centi_degrees)
{
        printf("set_angle_centi_degrees before (%lld) = %lld\n", centi_degrees, centi_degrees);

    centi_degrees = centi_degrees + (_offset_milli_degree/10);
    printf("set_angle_centi_degrees after (%lld) = %lld\n", centi_degrees, centi_degrees);
    // TODO: Maybe error check the input?
    // -90 is 125,000 is 1,000 us
    //   0 is 187,500 is 1,500 us
    // +90 is 250,000 is 2,000 us
    // UPDATE: The internet lied to me it seems the range is +- 90 degrees = 600-2400 us
    // So the new numbers are:
    // -90 is 75,000 is 600 us
    //   0 is 187,500 is 1,500 us
    // +90 is 300,000 is 2,400 us

    // Total servo range is 300,000 - 75,000 = 225,000

    // One step calculations
    // 125,000 total servo range
    // Degree input ranges from -9,000 to +9,000 (-90*100, +90*100)
    // 18,000 total range
    // 125,000/18,000 = 6.94444 per centi degree
    // or 694 per degree
    constexpr auto step_per_degree = 225000 / 180;
    constexpr auto midpoint_offset = 187500;

    // For better resolution use centi-degree then div by 100
    int64_t centi_degree_steps = step_per_degree * centi_degrees;
    int64_t angle_steps = midpoint_offset + (centi_degree_steps / 100);
    printf("set_angle_centi_degrees(%lld) = %lld\n", centi_degrees, angle_steps);
        pwm_set_chan_level(_slice_num, _channel, angle_steps / 64);
    }

    void servo::set_angle_milli_degrees(int64_t milli_degree)
    {
        printf("set_angle_milli_degrees(%lld)\n", milli_degree);
        set_angle_centi_degrees(milli_degree / 10);
    }

    void servo::turn_off()
    {
        pwm_set_chan_level(_slice_num, _channel, 0);
    }
    } //namespace drv
