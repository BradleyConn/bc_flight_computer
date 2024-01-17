#include "drv_gpio.h"
#include "pico/stdlib.h"
#include <stdio.h>

namespace drv
{
gpio::gpio(uint gpio, gpio_mode mode)
{
    gpio_init(gpio);
    switch (mode) {
        case gpio_mode::Input:
            gpio_set_dir(gpio, GPIO_IN);
            break;
        case gpio_mode::Output:
            gpio_set_dir(gpio, GPIO_OUT);
            break;
        case gpio_mode::InputPullUp:
            gpio_pull_up(gpio);
            gpio_set_dir(gpio, GPIO_IN);
            break;
        case gpio_mode::InputPullDown:
            gpio_pull_down(gpio);
            gpio_set_dir(gpio, GPIO_IN);
            break;
    }
    _gpio = gpio;
    _mode = mode;
}

gpio::~gpio()
{
    gpio_deinit(_gpio);
}

void gpio::set_gpio_output(gpio_state state)
{
    if (_mode != gpio_mode::Output) {
        printf("gpio::set_gpio_output() - ERROR: gpio mode is not output\n");
        return;
    }
    if (state == gpio_state::Low) {
        gpio_put(_gpio, 0);
    } else if (state == gpio_state::High) {
        gpio_put(_gpio, 1);
    }
}

gpio_state gpio::get_gpio_state()
{
    if (_mode != gpio_mode::Input) {
        printf("gpio::get_gpio_state() - ERROR: gpio mode is not input\n");
        return gpio_state::Low;
    }
    if (gpio_get(_gpio)) {
        return gpio_state::High;
    } else {
        return gpio_state::Low;
    }
}
} // namespace drv
