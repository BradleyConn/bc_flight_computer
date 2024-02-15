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
#include <vector>
//sqrt
#include <math.h>

int chute_ejection_test()
{
    //setup_default_uart();
    stdio_init_all();
    uart_init(uart0, 9600);
    printf("Hello, world! - This is Enos your flight computer speaking!\n");

    // init the barometer
    auto bmp280 = drv::bmp280(PICO_DEFAULT_SPI_SCLK_PIN_BMP280, PICO_DEFAULT_SPI_MISO_PIN_BMP280, PICO_DEFAULT_SPI_MOSI_PIN_BMP280,
                              PICO_DEFAULT_SPI_CS_PIN_BMP280, drv::bmp280::spi_module_1);

    bmp280.init();
    sleep_ms(1000);
    bmp280.calculate_baseline_pressure_and_altitude_cm();

    //init the gyro and accel
    auto bmi088 = drv::bmi088(PICO_DEFAULT_SPI_SCLK_PIN_BMI088, PICO_DEFAULT_SPI_MISO_PIN_BMI088, PICO_DEFAULT_SPI_MOSI_PIN_BMI088,
                              PICO_DEFAULT_SPI_ACCEL_CS_PIN_BMI088, PICO_DEFAULT_SPI_GYRO_CS_PIN_BMI088, drv::bmi088::spi_module_0,
                              PICO_DEFAULT_SPI_ACCEL_INT_PIN_BMI088, PICO_DEFAULT_SPI_GYRO_INT_PIN_BMI088);
    bmi088.init();
    //calibrate gyros
    bmi088.run_gyro_calibration();

    //init the buzzer
    auto buzzer = drv::buzzer(PICO_DEFAULT_BUZZER_PIN);
    buzzer.set_volume_percentage(0);

    //init servo E
    auto servo_E = drv::servo(PICO_DEFAULT_SERVO_E_PIN, drv::servo::servo_type::Analog, 0);
    servo_E.set_angle_centi_degrees(0 * 1000 / 10);

    //create a timekeeper
    auto timeKeeper = TimeKeeper();
    timeKeeper.mark();
    auto singleLoopTime = TimeKeeper();

    auto max = 0;

#if 0
    auto altitude_accumulator = 0;
    auto num_loops = 0;
    while (1) {
        // using the time keeper, wait until 500 ms has elapsed, then print the average altitude for that time
        if (timeKeeper.deltaTime_us() < 500000) {
            bmp280DatasetRaw bmp280RawData = bmp280.get_data_raw();
            bmp280DatasetConverted bmp280ConvertedData = bmp280.convert_data(bmp280RawData);
            altitude_accumulator += bmp280.pressure_Pa_to_relative_altitude_cm(bmp280ConvertedData.pressure_Pa);
            num_loops++;
        }
        else {
            timeKeeper.mark();
            printf("Average altitude: %d cm, %f feet\n", altitude_accumulator / num_loops, bmp280.cm_to_feet(altitude_accumulator / num_loops));

            altitude_accumulator = 0;
            num_loops = 0;
        }

        //try a moving average instead of just the average
        

    }
#endif

    //try a moving average instead of just the average
    auto num_loops = 0;
    //create an array to hold altitudes that are initialized to 0
    std::vector<int32_t> altitudes;
    constexpr auto num_averages = 5;
    altitudes.resize(num_averages, 0);
    singleLoopTime.mark();

    auto buzzerTime = TimeKeeper();
    buzzerTime.mark();
    int64_t last_average = 0;
    int64_t last_last_average = 0;
    auto magnitude = 0.0f;
    while (1) {
        //accel and gyro printing
        if (bmi088.accel_check_interrupt_data_ready()) {
            //read the data
            auto accelData = bmi088.accel_get_data_raw();
            auto accelDataConverted = bmi088.accel_convert_data(accelData);
            //calculate magnitude
            magnitude = sqrt(accelDataConverted.x_mg * accelDataConverted.x_mg + accelDataConverted.y_mg * accelDataConverted.y_mg +
                             accelDataConverted.z_mg * accelDataConverted.z_mg);

            //printf("Accel data milli g,,,,,,,,,,,,,,,,,,,,,,,, x: %ld, y: %ld, z: %ld, mag: %f\n", accelDataConverted.x_mg, accelDataConverted.y_mg, accelDataConverted.z_mg, magnitude);
            printf("A: %ld, %ld, %ld, m: %f\n", accelDataConverted.x_mg, accelDataConverted.y_mg, accelDataConverted.z_mg, magnitude);
            //read and clear the interrupt
            bmi088.accel_interrupt_reg_clear();
        }

        if (bmi088.gyro_check_interrupt_data_ready()) {
            auto gyroData = bmi088.gyro_get_data_raw();
            auto gyroDataConverted = bmi088.gyro_convert_data(gyroData);
            //printf("Gyro data milli deg/sec,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,x: %ld, y: %ld, z: %ld\n", gyroDataConverted.x_milli_degrees_per_sec, gyroDataConverted.y_milli_degrees_per_sec,
            //       gyroDataConverted.z_milli_degrees_per_sec);
            printf("G: %ld, %ld, %ld\n", gyroDataConverted.x_milli_degrees_per_sec, gyroDataConverted.y_milli_degrees_per_sec,
                   gyroDataConverted.z_milli_degrees_per_sec);
            bmi088.gyro_interrupt_reg_clear();
        }

        if (timeKeeper.deltaTime_us() < 39 * 1000 * num_averages && singleLoopTime.deltaTime_us() > 1000 * 38) {
            bmp280DatasetRaw bmp280RawData = bmp280.get_data_raw();
            bmp280DatasetConverted bmp280ConvertedData = bmp280.convert_data(bmp280RawData);
            //printf("Pressure: %d Pa\n", bmp280ConvertedData.pressure_Pa);
            //insert at the loop number
            altitudes[num_loops % num_averages] = bmp280.pressure_Pa_to_relative_altitude_cm(bmp280ConvertedData.pressure_Pa);
            //print that altitude
            //printf("Altitude: %d cm, %f feet\n", altitudes[num_loops % num_averages], bmp280.cm_to_feet(altitudes[num_loops % num_averages]));
            num_loops++;
            singleLoopTime.mark();
        } else if (timeKeeper.deltaTime_us() > 39 * 1000 * num_averages) {
            //print the average of the last num_averages altitudes
            int64_t average = 0;
            printf("Al: ");
            for (auto i : altitudes) {
                average += i;
                printf("%d, ", i);
            }
            puts("");
            average = average / num_averages;
            //time how long it takes to print this
            auto timeKeeperPrint = TimeKeeper();
            timeKeeperPrint.mark();
            printf("\n\nAv %lld cm, %f ft\n", average, bmp280.cm_to_feet(average));
            auto elapsed = timeKeeperPrint.deltaTime_us();
            printf("Time to print: %llu us\n", elapsed);
            //printf("looptime = %llu ms\n", timeKeeper.deltaTime_us() / 1000);
            //printf("num_loops = %d, time per loop = %llu us\n", num_loops, (timeKeeper.deltaTime_us() / num_loops)/1000);
            //print the whole array
            /*for (auto i : altitudes) {
                printf("%d, ", i);
            }*/

            timeKeeper.mark();
            //num_loops = 0;

            // If we're accelerating
            if (magnitude > 500) {
                max = average;
            } else {
                //on the way up
                if (max < average) {
                    max = average;
                } else {
                    // on the way down but double check it's not just noise
                    if (average < last_average /*&& last_average < last_last_average*/) {
                        printf("\n\n-------------------------------------Firing ejection charge\n");
                        servo_E.set_angle_centi_degrees(45 * 1000 / 10);
                        buzzer.set_volume_percentage(100);
                        buzzerTime.mark();
                        max = 0;
                        if (average < 0) {
                            max = average;
                        }
                    }
                }
            }
            if (buzzerTime.deltaTime_us() > 5 * 1000 * 1000) {
                buzzer.set_volume_percentage(0);
            }
            last_last_average = last_average;
            last_average = average;
        }
    }
}