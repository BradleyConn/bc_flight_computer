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

    // Tell GPIO 0 and 1 they are allocated to the PWM
    gpio_set_function(PICO_DEFAULT_LED_PIN_RED, GPIO_FUNC_PWM);
    gpio_set_function(PICO_DEFAULT_LED_PIN_GREEN, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN_RED);

    // Set period of 4 cycles (0 to 3 inclusive)
    pwm_set_wrap(slice_num, 160000);
    pwm_set_wrap(slice_num+1, 160000);
    // Set channel A output high for one cycle before dropping
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 3);
    pwm_set_chan_level(slice_num+1, PWM_CHAN_A, 3);
    // Set initial B output high for three cycles before dropping
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 3);
    pwm_set_chan_level(slice_num+1, PWM_CHAN_B, 3);

    // Set the PWM running
    pwm_set_enabled(slice_num, true);
    pwm_set_enabled(slice_num+1, true);
    /// \end::setup_pwm[]
    uint32_t pwm_red = 0;
    uint32_t pwm_green = 0;
    int x =-5;
    for(int i=0; i<400;) {
	    i++;
	    i=i%400;
	    if (i>200) {
		    pwm_green = i-200;
		    pwm_red = 400-i;
	    }
	    else {
		    pwm_green = 200-i;
		    pwm_red = i;
	    }
	    if(i==10) {
		    x++;
		}
	    if(i==0) {
		    servo_E.set_angle_centi_degrees(x*3000);
	    }
	    if(x==5) { x =-5;}
		
    // Set channel A output high for one cycle before dropping
    pwm_set_chan_level(slice_num, PWM_CHAN_A, pwm_red);
    pwm_set_chan_level(slice_num+1, PWM_CHAN_A, pwm_green);
    // Set initial B output high for three cycles before dropping
    pwm_set_chan_level(slice_num, PWM_CHAN_B, pwm_red);
    pwm_set_chan_level(slice_num+1, PWM_CHAN_B, pwm_green);
    for (volatile int j =0; j<10000; j++){}
    }

    // Note we could also use pwm_set_gpio_level(gpio, x) which looks up the
    // correct slice and channel for a given GPIO.
}
#endif
