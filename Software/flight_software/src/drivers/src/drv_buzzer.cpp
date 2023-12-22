#include "drv_buzzer.h"
#include "hardware/pwm.h"

namespace drv
{

buzzer::buzzer(uint gpio) : _gpio(gpio)
{
    gpio_set_function(_gpio, GPIO_FUNC_PWM);

    _slice_num = pwm_gpio_to_slice_num(_gpio);
    _channel = pwm_gpio_to_channel(_gpio);

    // servo takes a uint16_t which is 65535 which can fit 46295
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, 1);
    pwm_init(_slice_num, &config, true);

    set_frequency_hz(2700);
    set_volume_percentage(0);
    // Set the PWM running
    pwm_set_enabled(_slice_num, true);
}

buzzer::~buzzer()
{
    pwm_set_enabled(_slice_num, false);
}

void buzzer::set_frequency_hz(uint32_t frequency)
{
    // freq to period
    // 1/freq = period
    uint32_t period = 1E9 / frequency; // Convert frequency to period in ns
    uint32_t wrap = period / 8;        // Convert period to wrap value, 8ns is 125mhz

    pwm_set_wrap(_slice_num, wrap);
    _wrap_value_set = wrap;
}

void buzzer::set_volume_percentage(uint8_t volume_percent)
{
    // extra /2 because 50% duty cycle is the loudest
    auto level = _wrap_value_set * volume_percent / 100 / 2;
    pwm_set_chan_level(_slice_num, _channel, level);
}

}; // namespace drv