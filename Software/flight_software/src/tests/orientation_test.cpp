#include "orientation_test.h"
#include "../../bsp/enos.h"
#include "../../libs/Orientation/Orientation/Orientation.h"

#include "../drivers/inc/drv_bmi088.h"
#include "../system/inc/telemetry_container.h"
#include "../system/inc/time_keeper.h"
#include "orientation_test.h"
#include <stdio.h>

#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105
void orientation_test()
{
    Orientation orientation;
    EulerAngles eulerAngles;

    setup_default_uart();
    printf("Hello, world! - This is Enos your flight computer speaking!\n");

    auto bmi088 = drv::bmi088(PICO_DEFAULT_SPI_SCLK_PIN_BMI088, PICO_DEFAULT_SPI_MISO_PIN_BMI088, PICO_DEFAULT_SPI_MOSI_PIN_BMI088,
                              PICO_DEFAULT_SPI_ACCEL_CS_PIN_BMI088, PICO_DEFAULT_SPI_GYRO_CS_PIN_BMI088, drv::bmi088::spi_module_0,
                              PICO_DEFAULT_SPI_ACCEL_INT_PIN_BMI088, PICO_DEFAULT_SPI_GYRO_INT_PIN_BMI088);
    puts("Init bmi088!");
    bmi088.init();

    // Do some beeping to signify waiting for motionless
    // Determine if the IMU is motionless
    puts("Waiting for motionless...");
    // Then get a true upwards via accelerometer


    // Calibrate the gyros
    puts("Calibrating gyros...");
    bmi088.run_gyro_calibration();
    puts("Calibrated gyros!");
    // get calibration values
    auto gyroCalibrationValues = bmi088.get_gyro_calibration_values();
    auto gyroXOffset_f = ((float)gyroCalibrationValues.x_milli_degrees_per_sec) / 1000.0;
    auto gyroYOffset_f = ((float)gyroCalibrationValues.y_milli_degrees_per_sec) / 1000.0;
    auto gyroZOffset_f = ((float)gyroCalibrationValues.z_milli_degrees_per_sec) / 1000.0;

    printf("Gyro offsets: %f, %f, %f\n", gyroXOffset_f, gyroYOffset_f, gyroZOffset_f);

    auto timeKeeperStartOfWorld = TimeKeeper();
    timeKeeperStartOfWorld.mark();
    auto timeKeeper = TimeKeeper();
    timeKeeper.mark();
    auto timeKeeperPrint = TimeKeeper();
    timeKeeperPrint.mark();

    auto loopcount = 0;

    while (1) {
        loopcount++;
        while (bmi088.gyro_check_interrupt_data_ready() == false) {
            //printf("Waiting for gyro data ready\n");
        }
        //read and clear the interrupt
        bmi088.gyro_interrupt_reg_clear();

        float dtOrientation =
            (float)(timeKeeper.deltaTime_us()) / 1000000.; // Finds elapsed microseconds since last update, converts to float, and converts to seconds
        timeKeeper.mark();
        /*
        This is where the magic actually happens

        The order of your axis measurements (x, y, z) will depend on your sensor, your reference frame, and your IMU library of choice
        Swap & invert your gyro measurements so that .update() is called with (yaw, pitch, roll, dt) in that order

        All gyro measurements must be measured right-handed (positive = yaw left, pitch down, roll right) and coverted to radians/sec
        */

        bmi088DatasetRaw bmi088RawData = bmi088.get_data_raw();
        bmi088DatasetConverted bmi088ConvertedData = bmi088.convert_data(bmi088RawData);
        auto gyroX = bmi088ConvertedData.gyro_data_converted.x_milli_degrees_per_sec;
        auto gyroY = bmi088ConvertedData.gyro_data_converted.y_milli_degrees_per_sec;
        auto gyroZ = bmi088ConvertedData.gyro_data_converted.z_milli_degrees_per_sec;
        // Add the offset then convert from mDeg to Deg
        auto gyroX_f = ((float)gyroX) - gyroXOffset_f;
        auto gyroY_f = ((float)gyroY) - gyroYOffset_f;
        auto gyroZ_f = ((float)gyroZ) - gyroZOffset_f;
        gyroX_f = gyroX_f / 1000.0;
        gyroY_f = gyroY_f / 1000.0;
        gyroZ_f = gyroZ_f / 1000.0;
        orientation.update(gyroX_f * DEG_TO_RAD, gyroY_f * DEG_TO_RAD, gyroZ_f * DEG_TO_RAD,
                           dtOrientation); // '* DEG_TO_RAD' after all gyro functions if they return degrees/sec
        auto oriMeasure = orientation.toEuler();

        if (timeKeeperPrint.deltaTime_us() > 500000) {
            timeKeeperPrint.mark();
            printf("Loopcount = %lu\n", loopcount);
            timeKeeperStartOfWorld.printTimeMS();
            timeKeeper.printTimeuS();
            printf("Yaw: %f ", oriMeasure.yaw * RAD_TO_DEG);
            printf(", Pitch %f ", oriMeasure.pitch * RAD_TO_DEG);
            printf(", Roll: %f\n", oriMeasure.roll * RAD_TO_DEG);

            printf("GyroX: %d ", gyroX);
            printf(", GyroY %d ", gyroY);
            printf(", GyroZ: %d\n", gyroZ);
            printf("GyroX_f: %f ", gyroX_f);
            printf(", GyroY_f %f ", gyroY_f);
            printf(", GyroZ_f: %f\n", gyroZ_f);
            printf("accelX: %d ", bmi088ConvertedData.accel_data_converted.x_mg);
            printf(", accelY %d ", bmi088ConvertedData.accel_data_converted.y_mg);
            printf(", accelZ: %d\n", bmi088ConvertedData.accel_data_converted.z_mg);
        }
    }
}
