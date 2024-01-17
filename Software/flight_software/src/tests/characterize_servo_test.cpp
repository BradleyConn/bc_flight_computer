#include "../../bsp/enos.h"
#include "../drivers/inc/drv_bmi088.h"
#include "../drivers/inc/drv_bmp280.h"
#include "../drivers/inc/drv_buzzer.h"
#include "../drivers/inc/drv_flash.h"
#include "../drivers/inc/drv_led.h"
#include "../drivers/inc/drv_servo.h"
#include "../system/inc/telemetry_container.h"
#include "../system/inc/thrust_curves/thrust_curve_E9.h"
#include "../system/inc/time_keeper.h"
#include <stdio.h>

int characterize_servo_test()
{
    setup_default_uart();
    printf("Hello, world! - This is Enos your flight computer speaking!\n");
    auto timeKeeperStartOfWorld = TimeKeeper();
    timeKeeperStartOfWorld.mark();
    timeKeeperStartOfWorld.printTimeuS();
    auto timeKeeperLaunch = TimeKeeper();

    auto servo_E = drv::servo(PICO_DEFAULT_SERVO_E_PIN, drv::servo::servo_type::Analog, 0);

    //make an array to store the results of the servo behavior
    //make a loop to run the servo behavior 100 times
    uint64_t servoBehavior[100];
    for (int i = 0; i < 100; i++) {
        //create the logic to start a timer, set a servo postion, and check when the servo pushes a button, and records the time, to characterise the servo behavior. Use the drivers above
        // Then print out the standard deviation of the servo behavior
        servo_E.set_angle_centi_degrees(0);

        //start timer
        timeKeeperLaunch.mark();
        //set the servo angle +3 degrees
        servo_E.set_angle_centi_degrees(300);
        //wait for the servo to push the button
        while (1) {
            if (gpio_get(PICO_DEFAULT_SERVO_A_PIN)) {
                break;
            }
        }
        //record the time
        auto timeServoPushedButton = timeKeeperLaunch.deltaTime_us();
        // reset servo
        servo_E.set_angle_centi_degrees(0);
        //print the time
        printf("timeServoPushedButton = %llu\n", timeServoPushedButton);
        //store the time in the array
        servoBehavior[i] = timeServoPushedButton;
        //give it one second to reset
        sleep_ms(1000);
    }
    //print the standard deviation of the servo behavior
    // calculate it first
    uint64_t servoBehavior_mean = 0;
    for (int i = 0; i < 100; i++) {
        servoBehavior_mean += servoBehavior[i];
    }
    servoBehavior_mean /= 100;
    printf("servoBehavior_mean uS = %llu\n", servoBehavior_mean);
    uint64_t servoBehavior_std_dev = 0;
    for (int i = 0; i < 100; i++) {
        servoBehavior_std_dev += (servoBehavior[i] - servoBehavior_mean) * (servoBehavior[i] - servoBehavior_mean);
    }
    servoBehavior_std_dev /= 100;
    printf("servoBehavior_std_dev uS = %llu\n", servoBehavior_std_dev);
    // print the fastest and slowest servo behavior
    uint64_t servoBehavior_fastest = 0;
    uint64_t servoBehavior_slowest = 0;
    for (int i = 0; i < 100; i++) {
        if (servoBehavior[i] < servoBehavior_fastest) {
            servoBehavior_fastest = servoBehavior[i];
        }
        if (servoBehavior[i] > servoBehavior_slowest) {
            servoBehavior_slowest = servoBehavior[i];
        }
    }
    printf("servoBehavior_fastest uS = %llu\n", servoBehavior_fastest);
    printf("servoBehavior_slowest uS = %llu\n", servoBehavior_slowest);

    servo_E.turn_off();
    while (1)
        ;
}