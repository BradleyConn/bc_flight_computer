#include <stdio.h>
#include "pico/stdlib.h"

#if 0
int main() {
    setup_default_uart();
    while(1){
        printf("Hello, world!\n");
    }
    return 0;
}
#endif

#if 1

/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Output PWM signals on pins 0 and 1

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "../bsp/enos.h"
#include "drivers/inc/drv_servo.h"
#include "drivers/inc/drv_led.h"
#include "drivers/inc/drv_bmp280.h"
#include "drivers/inc/drv_bmi088.h"

int main()
{
    setup_default_uart();
    printf("Hello, world! - This is Enos your flight computer speaking!\n");
    /// \tag::setup_pwm[]
    //
    // sys clock set by oscillator (12mhz) * (fbdiv=100)
    // then 1200/ (postdiv1=6 * postdiv2=2) = 125
    //
    // set fsys to 125
    // then 125mhz gives 8ns period
    // 20ms/8ns = 2,500,000 = "wrap" (minus 1 because 0 indexed)
    // then 1.5ms/8ns = 187,500 = "level" (minus 1 because 0 indexed)
    // auto servo_E = drv::servo(24, drv::servo::servo_type::Analog, 0);
    // auto servo_D = drv::servo(22, drv::servo::servo_type::Analog, 0);
    // auto servo_C = drv::servo(21, drv::servo::servo_type::Analog, 0);
    // auto servo_B = drv::servo(18, drv::servo::servo_type::Analog, 0);
    // auto servo_A = drv::servo(16, drv::servo::servo_type::Analog, 0);
    // auto led_r = drv::pwm_led(PICO_DEFAULT_LED_PIN_RED,  50);
    // auto led_g = drv::pwm_led(PICO_DEFAULT_LED_PIN_GREEN, 50);
    auto bmp280 = drv::bmp280(10, 8, 11, 9, drv::bmp280::spi_module_1);
    auto bmi088 = drv::bmi088(2, 0, 3, 1, 5, drv::bmi088::spi_module_0);

    puts("Init bmp280!");
    bmp280.init();
    // bmp280.forever_test();
    puts("Init bmi088!");
    bmi088.init();
    puts("Init bmi088!");

    bmi088.init();

    while (1)
    {
        bmi088.getData();
        for (volatile int j = 0; j < 1000000; j++)
        {
        }
    }

    while (1)
    {
        uint32_t pwm_red = 0;
        uint32_t pwm_green = 0;
        int x = -5;
        for (int i = 0; i < 50; i++)
        {
            if (i > 25)
            {
                pwm_green = i - 25;
                pwm_red = 50 - i;
            }
            else
            {
                pwm_green = 25 - i;
                pwm_red = i;
            }
            if (i == 10)
            {
                x++;
            }
            if (i == 0)
            {
                // servo_E.set_angle_centi_degrees(x*300);
                // servo_D.set_angle_centi_degrees(x*300);
                // servo_C.set_angle_centi_degrees(x);
                // servo_B.set_angle_centi_degrees(x*-300);
                // servo_A.set_angle_centi_degrees(x*-300);
            }
            if (x == 5)
            {
                x = -5;
            }
            // led_r.set_pwm(pwm_red);
            // led_g.set_pwm(pwm_green);
            for (volatile int j = 0; j < 1000000; j++)
            {
            }
        }
    }

    // Note we could also use pwm_set_gpio_level(gpio, x) which looks up the
    // correct slice and channel for a given GPIO.
}
#endif
