#include "pico/stdlib.h"
#include <stdio.h>

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

#include "../bsp/enos.h"
#include "control_logic/inc/time_keeper.h"
#include "control_logic/inc/thrust_curve_E12.h"
#include "drivers/inc/drv_bmi088.h"
#include "drivers/inc/drv_bmp280.h"
#include "drivers/inc/drv_buzzer.h"
#include "drivers/inc/drv_led.h"
#include "drivers/inc/drv_servo.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

int main()
{
    setup_default_uart();
    printf("Hello, world! - This is Enos your flight computer speaking!\n");
    auto timeKeeperStartOfWorld = TimeKeeper();
    timeKeeperStartOfWorld.mark();
    timeKeeperStartOfWorld.printTimeuS();
    auto timeKeeperLaunch = TimeKeeper();

    // sys clock set by oscillator (12mhz) * (fbdiv=100)
    // then 1200/ (postdiv1=6 * postdiv2=2) = 125
    //
    // set fsys to 125
    // then 125mhz gives 8ns period
    // 20ms/8ns = 2,500,000 = "wrap" (minus 1 because 0 indexed)
    // then 1.5ms/8ns = 187,500 = "level" (minus 1 because 0 indexed)
    auto servo_E = drv::servo(PICO_DEFAULT_SERVO_E_PIN, drv::servo::servo_type::Analog, 0);
    auto servo_D = drv::servo(PICO_DEFAULT_SERVO_D_PIN, drv::servo::servo_type::Analog, 0);
    auto servo_C = drv::servo(PICO_DEFAULT_SERVO_C_PIN, drv::servo::servo_type::Analog, 0);
    auto servo_B = drv::servo(PICO_DEFAULT_SERVO_B_PIN, drv::servo::servo_type::Analog, 0);
    auto servo_A = drv::servo(PICO_DEFAULT_SERVO_A_PIN, drv::servo::servo_type::Analog, 0);
    auto led_r = drv::pwm_led(PICO_DEFAULT_LED_PIN_RED,  50);
    auto led_g = drv::pwm_led(PICO_DEFAULT_LED_PIN_GREEN, 50);
    auto buzzer = drv::buzzer(PICO_DEFAULT_BUZZER_PIN);
    auto bmp280 = drv::bmp280(PICO_DEFAULT_SPI_SCLK_PIN_BMP280, PICO_DEFAULT_SPI_MISO_PIN_BMP280, PICO_DEFAULT_SPI_MOSI_PIN_BMP280,
                              PICO_DEFAULT_SPI_CS_PIN_BMP280, drv::bmp280::spi_module_1);
    auto bmi088 = drv::bmi088(PICO_DEFAULT_SPI_SCLK_PIN_BMI088, PICO_DEFAULT_SPI_MISO_PIN_BMI088, PICO_DEFAULT_SPI_MOSI_PIN_BMI088,
                              PICO_DEFAULT_SPI_ACCEL_CS_PIN_BMI088, PICO_DEFAULT_SPI_GYRO_CS_PIN_BMI088, drv::bmi088::spi_module_0);
    // Todo: This should probably be the IThrustCurve interface. Keep it consistent for now with everything else.
    ThrustCurveE12 thrustCurve = ThrustCurveE12();

    puts("Init bmp280!");
    bmp280.init();
    //bmp280.forever_test();
    puts("Init bmi088!");
    bmi088.init();
    //buzzer.set_frequency(2700);



    while (1) {
        printf("Loop!\n");
        timeKeeperStartOfWorld.printTimeuS();
        auto bmi088RawData = bmi088.get_data_raw();
        puts("raw");
        bmi088.print_data_raw(bmi088RawData);
        auto bmi088ConvertedData = bmi088.convert_data(bmi088RawData);
        puts("converted");
        //bmi088.print_data_converted(bmi088ConvertedData);
        bmi088.print_data_converted_floats(bmi088ConvertedData);

        bmp280DatasetRaw bmp280RawData = bmp280.get_data_raw();
        bmp280DatasetConverted bmp280ConvertedData = bmp280.convert_data(bmp280RawData);
        bmp280.print_data_converted(bmp280ConvertedData);
        uint32_t pwm_red = 0;
        uint32_t pwm_green = 0;
        int x = -5;
        for (int i = 0; i < 50; i++) {
            if (i > 25) {
                pwm_green = i - 25;
                pwm_red = 50 - i;
            } else {
                pwm_green = 25 - i;
                pwm_red = i;
            }
            if (i == 10) {
                x++;
            }
            if (i == 0) {
                // servo_E.set_angle_centi_degrees(x*300);
                // servo_D.set_angle_centi_degrees(x*300);
                // servo_C.set_angle_centi_degrees(x);
                // servo_B.set_angle_centi_degrees(x*-300);
                // servo_A.set_angle_centi_degrees(x*-300);
            }
            if (x == 5) {
                x = -5;
            }
            // led_r.set_pwm(pwm_red);
            // led_g.set_pwm(pwm_green);
            for (volatile int j = 0; j < 1000000; j++) {
            }
        }
    }

    // Note we could also use pwm_set_gpio_level(gpio, x) which looks up the
    // correct slice and channel for a given GPIO.
}
#endif
