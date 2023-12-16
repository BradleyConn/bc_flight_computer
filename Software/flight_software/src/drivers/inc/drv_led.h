#ifndef _DRV_LED_H
#define _DRV_LED_H

#include "pico/stdlib.h"

namespace drv
{

class pwm_led
{
public:
    pwm_led(uint gpio, uint8_t starting_pwm_percent);
    ~pwm_led();

    void set_pwm(uint8_t percent);

private:
    uint _gpio;
    uint _channel;
    uint _slice_num;
}; // class pwm_led

}; // namespace drv
#endif
