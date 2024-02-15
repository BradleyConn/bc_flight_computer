#include "../bsp/enos.h"
#include "drivers/inc/drv_bmi088.h"
#include "drivers/inc/drv_bmp280.h"
#include "drivers/inc/drv_buzzer.h"
#include "drivers/inc/drv_flash.h"
#include "drivers/inc/drv_led.h"
#include "drivers/inc/drv_servo.h"
#include "system/inc/telemetry_container.h"
#include "system/inc/thrust_curves/thrust_curve_E9.h"
#include "system/inc/time_keeper.h"
#include "tests/characterize_servo_test.h"
#include "tests/orientation_test.h"
#include "tests/chute_ejection_test.h"
#include <stdio.h>

int main()
{
    chute_ejection_test();
    //characterize_servo_test();
    orientation_test2();
    #if 0
    setup_default_uart();
    printf("Hello, world! - This is Enos your flight computer speaking!\n");
    auto timeKeeperStartOfWorld = TimeKeeper();
    timeKeeperStartOfWorld.mark();
    timeKeeperStartOfWorld.printTimeuS();
    auto timeKeeperLaunch = TimeKeeper();

    auto flash = drv::FlashDriver();
    /*{
        // read the last session data and use the telemetry container to print it
        if (flash.has_previous_session()) {
            printf("FlashDriver::FlashDriver() - found a previous session, print data using the telemetry container\n");
            flash.dump_log_last_session();
            auto data = flash.get_log_ptr_previous_session();
            data += 2;
            printf("print last sesh\n");
            //print some of it
            for (int i = 0; i < 100; i++) {
                printf("%02x ", data[i]);
            }
            printf("\n");

            auto telemetry_container = sys::TelemetryContainer();
            telemetry_container.setPackagedRawBytes(data);
            telemetry_container.printPackagedTelemetryData();
            telemetry_container.printRawLogBytes();
            printf("DONE!\n\n");
        }
    }*/
    auto telemetry_container = sys::TelemetryContainer();

    auto servo_E = drv::servo(PICO_DEFAULT_SERVO_E_PIN, drv::servo::servo_type::Analog, 0);
    auto servo_D = drv::servo(PICO_DEFAULT_SERVO_D_PIN, drv::servo::servo_type::Analog, 0);
    auto servo_C = drv::servo(PICO_DEFAULT_SERVO_C_PIN, drv::servo::servo_type::Analog, 0);
    auto servo_B = drv::servo(PICO_DEFAULT_SERVO_B_PIN, drv::servo::servo_type::Analog, 0);
    auto servo_A = drv::servo(PICO_DEFAULT_SERVO_A_PIN, drv::servo::servo_type::Analog, 0);
    auto led_r = drv::pwm_led(PICO_DEFAULT_LED_PIN_RED, 2);
    auto led_g = drv::pwm_led(PICO_DEFAULT_LED_PIN_GREEN, 2);
    auto buzzer = drv::buzzer(PICO_DEFAULT_BUZZER_PIN);
    auto bmp280 = drv::bmp280(PICO_DEFAULT_SPI_SCLK_PIN_BMP280, PICO_DEFAULT_SPI_MISO_PIN_BMP280, PICO_DEFAULT_SPI_MOSI_PIN_BMP280,
                              PICO_DEFAULT_SPI_CS_PIN_BMP280, drv::bmp280::spi_module_1);
    auto bmi088 = drv::bmi088(PICO_DEFAULT_SPI_SCLK_PIN_BMI088, PICO_DEFAULT_SPI_MISO_PIN_BMI088, PICO_DEFAULT_SPI_MOSI_PIN_BMI088,
                              PICO_DEFAULT_SPI_ACCEL_CS_PIN_BMI088, PICO_DEFAULT_SPI_GYRO_CS_PIN_BMI088, drv::bmi088::spi_module_0,
                              PICO_DEFAULT_SPI_ACCEL_INT_PIN_BMI088, PICO_DEFAULT_SPI_GYRO_INT_PIN_BMI088);
    // Todo: This should probably be the IThrustCurve interface. Keep it consistent for now with everything else.
    ThrustCurveE9 thrust_curve = ThrustCurveE9();

    puts("Init bmp280!");
    bmp280.init();
    //bmp280.calculate_baseline_pressure_and_altitude_cm();
    //bmp280.forever_test();
    puts("Init bmi088!");
    bmi088.init();
    bmi088.run_gyro_calibration();
    auto gyroCalibrationValues = bmi088.get_gyro_calibration_values();

    buzzer.set_frequency_hz(2700);
    //buzzer.set_volume_percentage(2);

    //servo_E.turn_off();
    //while (1)
    //    ;

    uint32_t loopcount = 0;
    auto timeKeeperLoopTime = TimeKeeper();
    timeKeeperLoopTime.mark();
    while (1) {
        loopcount++;
        if (loopcount % 1000 == 0) {
            printf("Loopcount = %lu\n", loopcount);
            timeKeeperLoopTime.printTimeuS();
            timeKeeperLoopTime.mark();
        }
        while (bmi088.accel_check_interrupt_data_ready() == false) {
            //printf("Waiting for accel data ready\n");
        }
        //read and clear the interrupt
        bmi088.accel_interrupt_reg_clear();

        while (bmi088.gyro_check_interrupt_data_ready() == false) {
            //printf("Waiting for gyro data ready\n");
        }
        //read and clear the interrupt
        bmi088.gyro_interrupt_reg_clear();

        bmi088DatasetRaw bmi088RawData = bmi088.get_data_raw();
        bmi088DatasetConverted bmi088ConvertedData = bmi088.convert_data(bmi088RawData);
    }

    while (1) {
        //printf("Loopcount = %lu\n", loopcount);
        loopcount++;
        while (bmi088.accel_check_interrupt_data_ready() == false) {
            printf("Waiting for accel data ready\n");
        }
        //timeKeeperStartOfWorld.printTimeuS();
        auto timeKeeperA = TimeKeeper();
        timeKeeperA.mark();
        //35us
        auto bmi088RawData = bmi088.get_data_raw();
        //timeKeeperA.printTimeuS();
        //puts("raw");
        //bmi088.print_data_raw(bmi088RawData);
        timeKeeperA.mark();
        //10us
        auto bmi088ConvertedData = bmi088.convert_data(bmi088RawData);
        //timeKeeperA.printTimeuS();
        //puts("converted");
        //bmi088.print_data_converted(bmi088ConvertedData);
        //bmi088.print_data_converted_floats(bmi088ConvertedData);

        //puts("bmp280");
        timeKeeperA.mark();
        // 10us
        bmp280DatasetRaw bmp280RawData = bmp280.get_data_raw();
        //timeKeeperA.printTimeuS();
        timeKeeperA.mark();
        //2us
        bmp280DatasetConverted bmp280ConvertedData = bmp280.convert_data(bmp280RawData);
        //timeKeeperA.printTimeuS();
        //bmp280.print_data_converted(bmp280ConvertedData);

        //printf("\n\n");
        telemetry_container.setBMI088DatasetRaw(bmi088RawData);
        telemetry_container.setBMI088DatasetConverted(bmi088ConvertedData);
        telemetry_container.setBMP280DatasetRaw(bmp280RawData);
        telemetry_container.setBMP280DatasetConverted(bmp280ConvertedData);
        telemetry_container.setTimeData1(timeKeeperStartOfWorld.deltaTime_us());
        telemetry_container.setTimeData2(timeKeeperLaunch.deltaTime_us());
        telemetry_container.setTimeData3(timeKeeperLaunch.deltaTime_us());
        //telemetry_container.printRawLogBytes();
        //telemetry_container.printPackagedTelemetryData();

        //flash.write_next_usable_page_size(telemetry_container.getPackagedRawBytes());

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
        }
        //sleep_ms(500);
        //servo_E.set_angle_centi_degrees(-9000 + (loopcount%3) * 9000);
        //buzzer.set_frequency_hz((loopcount % 50) * 100);
    }
    #endif
}