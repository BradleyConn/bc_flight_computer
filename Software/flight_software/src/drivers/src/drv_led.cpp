#include "../inc/drv_led.h"
#include "hardware/pwm.h"

namespace drv
{

pwm_led::pwm_led(uint gpio, uint8_t starting_pwm_percent) : _gpio(gpio)
{
    gpio_set_function(_gpio, GPIO_FUNC_PWM);

    _slice_num = pwm_gpio_to_slice_num(_gpio);
    _channel = pwm_gpio_to_channel(_gpio);

    // The max counter
    pwm_set_wrap(_slice_num, 100);
    set_pwm(starting_pwm_percent);
    // Set the PWM running
    pwm_set_enabled(_slice_num, true);
}

pwm_led::~pwm_led()
{
    pwm_set_enabled(_slice_num, false);
}

void pwm_led::set_pwm(uint8_t percent)
{
    pwm_set_chan_level(_slice_num, _channel, percent);
}

} //namespace drv
