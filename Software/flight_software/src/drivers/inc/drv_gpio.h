#ifndef _DRV_GPIO_H
#define _DRV_GPIO_H

#include "pico/stdlib.h"

namespace drv
{
enum class gpio_mode
{
    Input,
    Output,
    InputPullUp,
    InputPullDown,
};
enum class gpio_state
{
    Low,
    High,
};
class gpio
{
public:
    gpio(uint gpio, gpio_mode mode);
    ~gpio();

    void set_gpio_output(gpio_state state);
    gpio_state get_gpio_state();

private:
    uint _gpio;
    gpio_mode _mode;
}; // class gpio

}; // namespace drv
#endif
