#include "orientation_test.h"
#include "../../bsp/enos.h"
#include "../../libs/Orientation/Orientation/Orientation.h"

#include "../drivers/inc/drv_bmi088.h"
#include "../system/inc/telemetry_container.h"
#include "../system/inc/time_keeper.h"
#include "orientation_test.h"
#include <math.h>
#include <stdio.h>

#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105
void orientation_test()
{
    Orientation orientation;
    EulerAngles eulerAngles;

    setup_default_uart();
    //printf("Hello, world! - This is Enos your flight computer speaking!\n");

    auto bmi088 = drv::bmi088(PICO_DEFAULT_SPI_SCLK_PIN_BMI088, PICO_DEFAULT_SPI_MISO_PIN_BMI088, PICO_DEFAULT_SPI_MOSI_PIN_BMI088,
                              PICO_DEFAULT_SPI_ACCEL_CS_PIN_BMI088, PICO_DEFAULT_SPI_GYRO_CS_PIN_BMI088, drv::bmi088::spi_module_0,
                              PICO_DEFAULT_SPI_ACCEL_INT_PIN_BMI088, PICO_DEFAULT_SPI_GYRO_INT_PIN_BMI088);
    //puts("Init bmi088!");
    bmi088.init();

    // Do some beeping to signify waiting for motionless
    // Determine if the IMU is motionless
    //puts("Waiting for motionless...");
    // Then get a true upwards via accelerometer

    // Calibrate the gyros
    //puts("Calibrating gyros...");
    //bmi088.run_gyro_calibration();
    //puts("Calibrated gyros!");
    // get calibration values
    auto gyroCalibrationValues = bmi088.get_gyro_calibration_values();
    auto gyroXOffset_f = ((float)gyroCalibrationValues.x_milli_degrees_per_sec) / 1000.0;
    auto gyroYOffset_f = ((float)gyroCalibrationValues.y_milli_degrees_per_sec) / 1000.0;
    auto gyroZOffset_f = ((float)gyroCalibrationValues.z_milli_degrees_per_sec) / 1000.0;

    // get raw acell values
    bmi088DatasetRaw bmi088RawData = bmi088.get_data_raw();
    // get converted acell values
    bmi088DatasetConverted bmi088ConvertedData = bmi088.convert_data(bmi088RawData);
    // get gravity and call orientation.update_gravity()
    auto accelX = bmi088ConvertedData.accel_data_converted.x_mg;
    auto accelY = bmi088ConvertedData.accel_data_converted.y_mg;
    auto accelZ = bmi088ConvertedData.accel_data_converted.z_mg;
    auto accelX_f = ((float)accelX) / 1000.0;
    auto accelY_f = ((float)accelY) / 1000.0;
    auto accelZ_f = ((float)accelZ) / 1000.0;

    // Z = yaw, X = pitch, Y = roll

    //print the float values
    //printf("accelX_f: %f ", accelX_f);
    //printf(", accelY_f %f ", accelY_f);
    //printf(", accelZ_f: %f\n", accelZ_f);
    //orientation.updateGravity(accelX_f, accelY_f, accelZ_f);
    //orientation.update(0.0f,0.0f,0.0f, .1);
    //orientation.update(0.0f,0.0f,0.0f, .1);
    //orientation.update(0.0f,0.0f,0.0f, .1);

    //orientation.updateGravity(accelX_f, accelY_f, accelZ_f);
    //orientation.update(0.0f,0.0f,0.0f, .1);
    //orientation.orientation = orientation.worldGravity;
    //orientation.update(0.0f,0.0f,0.0f, .1);
    orientation.update(0.0f, 0.0f, 0.0f, .1);

    auto oriMeasure = orientation.toEuler();

    //printf("Loopcount = %lu\n", loopcount);
    //timeKeeperStartOfWorld.printTimeMS();
    //timeKeeper.printTimeuS();
    int sleeptime = 5;
    int num_iterations = 0;
    sleep_ms(sleeptime);

    printf("y%fyp%fpr%fr\n", oriMeasure.yaw * RAD_TO_DEG, oriMeasure.pitch * RAD_TO_DEG, oriMeasure.roll * RAD_TO_DEG);
    printf("y%fyp%fpr%fr\n", oriMeasure.yaw * RAD_TO_DEG, oriMeasure.pitch * RAD_TO_DEG, oriMeasure.roll * RAD_TO_DEG);

    sleep_ms(sleeptime);

    auto dtOrientation = 1.0f;
    auto gyroX_f = 0.0;
    auto gyroY_f = 0.0;
    auto gyroZ_f = 0.0;
    auto yaw_f = gyroZ_f;
    auto pitch_f = gyroY_f;
    auto roll_f = gyroX_f;
    auto magnitude = 90; // total asimouth angle change
    auto theta = 45;     // east to north angle
    yaw_f = magnitude * sinf(theta * DEG_TO_RAD);
    pitch_f = magnitude * cosf(theta * DEG_TO_RAD);
    roll_f = -0;
    puts("");
    yaw_f = 90;
    pitch_f = 0;
    roll_f = 0;
    printf("1A.Feed to orientation: yaw_f %f pitch_f %f roll_f %f\n", yaw_f, pitch_f, roll_f);
    orientation.update(yaw_f * DEG_TO_RAD, pitch_f * DEG_TO_RAD, roll_f * DEG_TO_RAD,
                       dtOrientation); // '* DEG_TO_RAD' after all gyro functions if they return degrees/sec
    oriMeasure = orientation.toEuler();

    printf("1B.Return back EULER: yaw %f, pitch %f, roll %f\n", oriMeasure.yaw * RAD_TO_DEG, oriMeasure.pitch * RAD_TO_DEG, oriMeasure.roll * RAD_TO_DEG);

    yaw_f = 0;
    pitch_f = -90;
    roll_f = 0;
    printf("2A.Feed to orientation: yaw_f %f pitch_f %f roll_f %f\n", yaw_f, pitch_f, roll_f);
    orientation.update(yaw_f * DEG_TO_RAD, pitch_f * DEG_TO_RAD, roll_f * DEG_TO_RAD,
                       dtOrientation); // '* DEG_TO_RAD' after all gyro functions if they return degrees/sec
    oriMeasure = orientation.toEuler();
    printf("2B.Return back EULER: yaw %f, pitch %f, roll %f\n", oriMeasure.yaw * RAD_TO_DEG, oriMeasure.pitch * RAD_TO_DEG, oriMeasure.roll * RAD_TO_DEG);

    yaw_f = 0;
    pitch_f = 0;
    roll_f = -90;
    printf("3A.Feed to orientation: yaw_f %f pitch_f %f roll_f %f\n", yaw_f, pitch_f, roll_f);
    orientation.update(yaw_f * DEG_TO_RAD, pitch_f * DEG_TO_RAD, roll_f * DEG_TO_RAD,
                       dtOrientation); // '* DEG_TO_RAD' after all gyro functions if they return degrees/sec
    oriMeasure = orientation.toEuler();
    printf("3B.Return back EULER: yaw %f, pitch %f, roll %f\n", oriMeasure.yaw * RAD_TO_DEG, oriMeasure.pitch * RAD_TO_DEG, oriMeasure.roll * RAD_TO_DEG);

    yaw_f = 0;
    pitch_f = 0;
    roll_f = -90;
    printf("4A.Feed to orientation: yaw_f %f pitch_f %f roll_f %f\n", yaw_f, pitch_f, roll_f);
    orientation.update(yaw_f * DEG_TO_RAD, pitch_f * DEG_TO_RAD, roll_f * DEG_TO_RAD,
                       dtOrientation); // '* DEG_TO_RAD' after all gyro functions if they return degrees/sec
    oriMeasure = orientation.toEuler();
    printf("4B.Return back EULER: yaw %f, pitch %f, roll %f\n", oriMeasure.yaw * RAD_TO_DEG, oriMeasure.pitch * RAD_TO_DEG, oriMeasure.roll * RAD_TO_DEG);

    yaw_f = 0;
    pitch_f = 0;
    roll_f = -90;
    printf("5A.Feed to orientation: yaw_f %f pitch_f %f roll_f %f\n", yaw_f, pitch_f, roll_f);
    orientation.update(yaw_f * DEG_TO_RAD, pitch_f * DEG_TO_RAD, roll_f * DEG_TO_RAD,
                       dtOrientation); // '* DEG_TO_RAD' after all gyro functions if they return degrees/sec
    oriMeasure = orientation.toEuler();
    printf("5B.Return back EULER: yaw %f, pitch %f, roll %f\n", oriMeasure.yaw * RAD_TO_DEG, oriMeasure.pitch * RAD_TO_DEG, oriMeasure.roll * RAD_TO_DEG);

    yaw_f = 0;
    pitch_f = 0;
    roll_f = -90;
    printf("6A.Feed to orientation: yaw_f %f pitch_f %f roll_f %f\n", yaw_f, pitch_f, roll_f);
    orientation.update(yaw_f * DEG_TO_RAD, pitch_f * DEG_TO_RAD, roll_f * DEG_TO_RAD,
                       dtOrientation); // '* DEG_TO_RAD' after all gyro functions if they return degrees/sec
    oriMeasure = orientation.toEuler();
    printf("6B.Return back EULER: yaw %f, pitch %f, roll %f\n", oriMeasure.yaw * RAD_TO_DEG, oriMeasure.pitch * RAD_TO_DEG, oriMeasure.roll * RAD_TO_DEG);

    puts("ha");

    for (int n = 1; n < num_iterations + 1; n++) {
        // Do yaw
        printf("y%fyp%fpr%fr\n", ((float)n / (float)num_iterations) * oriMeasure.yaw * RAD_TO_DEG, 0.0, 0.0);
        printf("y%fyp%fpr%fr\n", ((float)n / (float)num_iterations) * oriMeasure.yaw * RAD_TO_DEG, 0.0, 0.0);

        sleep_ms(sleeptime);
    }
    for (int n = 0; n < num_iterations; n++) {
        // Do pitch
        printf("y%fyp%fpr%fr\n", oriMeasure.yaw * RAD_TO_DEG, ((float)n / (float)num_iterations) * oriMeasure.pitch * RAD_TO_DEG, 0.0);
        printf("y%fyp%fpr%fr\n", oriMeasure.yaw * RAD_TO_DEG, ((float)n / (float)num_iterations) * oriMeasure.pitch * RAD_TO_DEG, 0.0);
        sleep_ms(sleeptime);
    }
    for (int n = 0; n < num_iterations; n++) {
        // Do roll
        printf("y%fyp%fpr%fr\n", oriMeasure.yaw * RAD_TO_DEG, oriMeasure.pitch * RAD_TO_DEG, ((float)n / (float)num_iterations) * oriMeasure.roll * RAD_TO_DEG);
        printf("y%fyp%fpr%fr\n", oriMeasure.yaw * RAD_TO_DEG, oriMeasure.pitch * RAD_TO_DEG, ((float)n / (float)num_iterations) * oriMeasure.roll * RAD_TO_DEG);

        sleep_ms(sleeptime);
    }

    printf("y%fyp%fpr%fr\n", oriMeasure.yaw * RAD_TO_DEG, oriMeasure.pitch * RAD_TO_DEG, oriMeasure.roll * RAD_TO_DEG);
    printf("y%fyp%fpr%fr\n", oriMeasure.yaw * RAD_TO_DEG, oriMeasure.pitch * RAD_TO_DEG, oriMeasure.roll * RAD_TO_DEG);

    float sinRoll = sin(-oriMeasure.roll); // NB MUST BE IN Radians !!!
    float cosRoll = cos(-oriMeasure.roll); // NB MUST BE IN Radians !!!
    float yPIDOutput = oriMeasure.pitch * RAD_TO_DEG;
    float zPIDOutput = oriMeasure.yaw * RAD_TO_DEG;
    float yServoAngle = (yPIDOutput * cosRoll) - (zPIDOutput * sinRoll);
    float zServoAngle = (yPIDOutput * sinRoll) + (zPIDOutput * cosRoll);
    printf("yServoAngle %f zServoAngle %f \n", yServoAngle, zServoAngle);
    printf("sinRoll %f cosRoll %f \n", sinRoll, cosRoll);
    printf("yPIDOutput %f zPIDOutput %f \n", yPIDOutput, zPIDOutput);

    orientation.update(zServoAngle * DEG_TO_RAD, yServoAngle * DEG_TO_RAD, 0 * DEG_TO_RAD,
                       dtOrientation); // '* DEG_TO_RAD' after all gyro functions if they return degrees/sec
    oriMeasure = orientation.toEuler();
    printf("y%fyp%fpr%fr\n", -oriMeasure.yaw * RAD_TO_DEG, -oriMeasure.pitch * RAD_TO_DEG, oriMeasure.roll * RAD_TO_DEG);

    while (1)
        ;

    //printf("Gyro offsets: %f, %f, %f\n", gyroXOffset_f, gyroYOffset_f, gyroZOffset_f);

    // get gravity and call orientation.update_gravity()

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

        if (timeKeeperPrint.deltaTime_us() > 100000) {
            timeKeeperPrint.mark();
            //printf("Loopcount = %lu\n", loopcount);
            //timeKeeperStartOfWorld.printTimeMS();
            //timeKeeper.printTimeuS();
            printf("y%fy", oriMeasure.yaw * RAD_TO_DEG);
            printf("p%fp", oriMeasure.pitch * RAD_TO_DEG);
            printf("r%fr\n", oriMeasure.roll * RAD_TO_DEG);
            /*
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
            printf(", accelZ: %d\n", bmi088ConvertedData.accel_data_converted.z_mg);*/
        }
        while (1)
            ;
    }
}

Orientation orientation;

void helper(float yaw_f, float pitch_f, float roll_f)
{
    printf("Feed to orientation ------> yaw_f %f pitch_f %f roll_f %f\n", yaw_f, pitch_f, roll_f);
    orientation.update(yaw_f * DEG_TO_RAD, pitch_f * DEG_TO_RAD, roll_f * DEG_TO_RAD,
                       1.0); // '* DEG_TO_RAD' after all gyro functions if they return degrees/sec
    auto oriMeasure = orientation.toEuler();

    printf("Return back EULER =========================================================== YAW: %f, PITCH: %f, ROLL: %f\n", oriMeasure.yaw * RAD_TO_DEG,
           oriMeasure.pitch * RAD_TO_DEG, oriMeasure.roll * RAD_TO_DEG);
}

void orientation_test2()
{
    setup_default_uart();

    puts("\n\nNew:");
    helper(75, 0, 0);
    helper(0, 45, 0);
    //helper(45, 0, 0);

    auto oriMeasure = orientation.toEuler();

    helper(0, 0, 0);

    auto yaw = oriMeasure.yaw;
    auto pitch = oriMeasure.pitch;
    auto roll = oriMeasure.roll;
    auto mag = acos(cos(yaw) * cos(pitch));
    printf("yaw = %f, pitch = %f, roll = %f\n", yaw * RAD_TO_DEG, pitch * RAD_TO_DEG, roll * RAD_TO_DEG);
    printf("mag = %f\n", mag * RAD_TO_DEG);
    auto rotation_offset = acos(-cos(mag)/sin(mag)*cos((M_PI/2)-pitch)/sin((M_PI/2)-pitch));
    printf("rotation_offset = %f\n", rotation_offset * RAD_TO_DEG);
    helper(0, 0, - rotation_offset * RAD_TO_DEG);
    helper(0, mag * RAD_TO_DEG, 0);

#if 0
    for (int i = 0; i < 636; i++) {
        helper(.1, .1, .1);
        oriMeasure = orientation.toEuler();

        if (oriMeasure.yaw * RAD_TO_DEG > 89) {
            printf(" i = %d\n", i);
            break;
        }
    }
#endif
#if 0
    //helper(89, 0, 0);
    //helper(0, 89, 0);
    helper(0, 0, 0);

    //auto oriMeasure = orientation.toEuler();
    auto yaw = oriMeasure.yaw;
    auto cos_yaw = cos(yaw);
    auto sin_yaw = sin(yaw);
    auto pitch = oriMeasure.pitch;
    auto cos_pitch = cos(pitch);
    auto sin_pitch = sin(pitch);
    auto roll = oriMeasure.roll;
    auto cos_roll = cos(roll);
    auto sin_roll = sin(roll);
    printf("yaw = %f, cos_yaw = %f, sin_yaw = %f\n", yaw, cos_yaw, sin_yaw);
    printf("pitch = %f, cos_pitch = %f, sin_pitch = %f\n", pitch, cos_pitch, sin_pitch);
    printf("roll = %f, cos_roll = %f, sin_roll = %f\n", roll, cos_roll, sin_roll);
    auto myGuessYaw = sin_yaw * cos_pitch;
    auto myGuessPitch = cos_yaw * sin_pitch;
    printf("myGuessYaw = %f, myGuessPitch = %f\n", myGuessYaw, myGuessPitch);
    float cosRoll = cos(-oriMeasure.roll);
    float sinRoll = sin(-oriMeasure.roll);

    //auto yPIDAngle = oriMeasure.pitch * RAD_TO_DEG;
    //auto zPIDAngle = oriMeasure.yaw * RAD_TO_DEG;

    auto magnitude = 1;                                    // total asimouth angle change
    auto theta = 45;                                       // east to north angle
    auto yPIDAngle = oriMeasure.pitch;//magnitude * sinf(theta * DEG_TO_RAD); // 0
    auto zPIDAngle = oriMeasure.yaw; //magnitude * cosf(theta * DEG_TO_RAD); // 90
    auto pitchServoAngle = yPIDAngle * cosRoll - zPIDAngle * sinRoll;
    auto yawServoAngle = yPIDAngle * sinRoll + zPIDAngle * cosRoll;

    printf("yaw = %f, pitch = %f, roll = %f\n", oriMeasure.yaw * RAD_TO_DEG, oriMeasure.pitch * RAD_TO_DEG, oriMeasure.roll * RAD_TO_DEG);
    printf("yPIDAngle = %f, zPIDAngle = %f\n", yPIDAngle, zPIDAngle);
    printf("pitchServoAngle = %f, yawServoAngle = %f\n", pitchServoAngle, yawServoAngle);
    printf("sinRoll = %f, cosRoll = %f\n", sinRoll, cosRoll);
    helper(0, 0, 89);
    helper(0, 0, 89);
    helper(0, 0, 89);
    helper(0, 0, 89);
#endif

    while (1)
        ;
}
