#include <stdio.h>
#include "pico/stdlib.h"

#if 0 
int main() {
    setup_default_uart();
    printf("Hello, world!\n");
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

int main() {
    /// \tag::setup_pwm[]
    //
    //sys clock set by oscillator (12mhz) * (fbdiv=100)
    //then 1200/ (postdiv1=6 * postdiv2=2) = 125
    //
    //set fsys to 125
    //then 125mhz gives 8ns period
    //20ms/8ns = 2,500,000 = "wrap" (minus 1 because 0 indexed)
    //then 1.5ms/8ns = 187,500 = "level" (minus 1 because 0 indexed)
    auto servo_E = drv::servo(24, PWM_CHAN_A, drv::servo::servo_type::Analog, 0);
    auto led_r = drv::pwm_led(PICO_DEFAULT_LED_PIN_RED, PWM_CHAN_B, 50);
    auto led_g = drv::pwm_led(PICO_DEFAULT_LED_PIN_GREEN, PWM_CHAN_B, 50);

    while(1){
        uint32_t pwm_red = 0;
        uint32_t pwm_green = 0;
        int x =-5;
        for(int i=0; i<50; i++) {
                if (i>25) {
            	    pwm_green = i-25;
            	    pwm_red = 50-i;
                }
                else {
            	    pwm_green = 25-i;
            	    pwm_red = i;
                }
                if(i==10) {
            	    x++;
            	}
                if(i==0) {
            	    servo_E.set_angle_centi_degrees(x*3000);
                }
                if(x==5) { x =-5;}
        led_r.set_pwm(pwm_red);
        led_g.set_pwm(pwm_green);
        for (volatile int j =0; j<1000000; j++){}
        }
    }

    // Note we could also use pwm_set_gpio_level(gpio, x) which looks up the
    // correct slice and channel for a given GPIO.
}
#endif
