#include "../bsp/enos.h"
#include "drivers/inc/drv_bmi088.h"
#include "drivers/inc/drv_bmp280.h"
#include "drivers/inc/drv_buzzer.h"
#include "drivers/inc/drv_flash.h"
#include "drivers/inc/drv_led.h"
#include "drivers/inc/drv_servo.h"
#include "system/inc/control_loop.h"
#include "system/inc/logger.h"
#include "system/inc/parachute.h"
#include "system/inc/state_detector.h"
#include "system/inc/telemetry_container.h"
#include "system/inc/thrust_curves/thrust_curve_E9.h"
#include "system/inc/time_keeper.h"
#include "tests/characterize_servo_test.h"
#include "tests/chute_ejection_test.h"
#include "tests/orientation_test.h"
#include <stdio.h>

//#define DEBUG

int main()
{
    //chute_ejection_test();
    //characterize_servo_test();
    //orientation_test2();

    setup_default_uart();
    stdio_init_all();
    printf("Hello, world! - This is Enos your flight computer speaking!\n");
    auto timeKeeperStartOfWorld = TimeKeeper();
    timeKeeperStartOfWorld.mark();
    timeKeeperStartOfWorld.printTimeuS();
    auto timeKeeperLaunch = TimeKeeper();

    //TODO: Put this in a config file
    constexpr uint32_t MAX_COAST_TIME_MS = 5000;
    // 5 ms
    constexpr uint32_t INTERRUPT_TIMEOUT_US = 5 * 1000;

    constexpr uint32_t LIFTOFF_ACCEL_THRESHOLD_MG = 1750;
    constexpr uint32_t MOTOR_BURNOUT_ACCEL_THRESHOLD_MG = 250;
    constexpr uint32_t APOGEE_ALTITUDE_THRESHOLD_CM = 5 * 100; // 5m
    constexpr uint32_t LANDING_ACCEL_THRESHOLD_MG = 700;
    constexpr int32_t PITCH_SERVO_OFFSET_MILLI_DEG = -18000;
    constexpr int32_t YAW_SERVO_OFFSET_MILLI_DEG = 6000;
    constexpr int32_t CONTROL_LOOP_UPDATE_RATE_US = 40 * 1000;
    constexpr float LEVER_ARM_M = 0.09525;
    constexpr float MMOI_KG_M2 = 0.00417804;
    auto default_buzzer_volume = 100;

    auto flash = drv::FlashDriver();
    flash.dump_full_log();
    //flash.reset_log();
    // TODO: Dump the entire log

    {
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
    }
    auto telemetry_container = sys::TelemetryContainer();
    auto logger = Logger(flash);

    // TODO: Pull these out into a config file...
    //auto servo_E = drv::servo(PICO_DEFAULT_SERVO_E_PIN, drv::servo::servo_type::Analog, 0);
    // The parachute servo
    auto servo_D_Parachute = drv::servo(PICO_DEFAULT_SERVO_D_PIN, drv::servo::servo_type::Analog, 0);
    // The pitch servo
    auto servo_C_Pitch = drv::servo(PICO_DEFAULT_SERVO_C_PIN, drv::servo::servo_type::Digital, PITCH_SERVO_OFFSET_MILLI_DEG, 0);
    auto servo_B = drv::servo(PICO_DEFAULT_SERVO_B_PIN, drv::servo::servo_type::Analog, 0);
    // The yaw servo
    auto servo_A_Yaw = drv::servo(PICO_DEFAULT_SERVO_A_PIN, drv::servo::servo_type::Digital, YAW_SERVO_OFFSET_MILLI_DEG, 0);

    auto led_r = drv::pwm_led(PICO_DEFAULT_LED_PIN_RED, 2);
    auto led_g = drv::pwm_led(PICO_DEFAULT_LED_PIN_GREEN, 2);
    auto buzzer = drv::buzzer(PICO_DEFAULT_BUZZER_PIN);
    auto bmp280 = drv::bmp280(PICO_DEFAULT_SPI_SCLK_PIN_BMP280, PICO_DEFAULT_SPI_MISO_PIN_BMP280, PICO_DEFAULT_SPI_MOSI_PIN_BMP280,
                              PICO_DEFAULT_SPI_CS_PIN_BMP280, drv::bmp280::spi_module_1);
    auto bmi088 = drv::bmi088(PICO_DEFAULT_SPI_SCLK_PIN_BMI088, PICO_DEFAULT_SPI_MISO_PIN_BMI088, PICO_DEFAULT_SPI_MOSI_PIN_BMI088,
                              PICO_DEFAULT_SPI_ACCEL_CS_PIN_BMI088, PICO_DEFAULT_SPI_GYRO_CS_PIN_BMI088, drv::bmi088::spi_module_0,
                              PICO_DEFAULT_SPI_ACCEL_INT_PIN_BMI088, PICO_DEFAULT_SPI_GYRO_INT_PIN_BMI088);

    auto state_detector = StateDetector(LIFTOFF_ACCEL_THRESHOLD_MG, MOTOR_BURNOUT_ACCEL_THRESHOLD_MG, APOGEE_ALTITUDE_THRESHOLD_CM, LANDING_ACCEL_THRESHOLD_MG);
    auto orientation_calculator = OrientationCalculator();
    auto control_loop = ControlLoop(orientation_calculator, servo_A_Yaw, servo_C_Pitch, CONTROL_LOOP_UPDATE_RATE_US, LEVER_ARM_M, MMOI_KG_M2);
    auto parachute = Parachute(servo_B);
    parachute.arm();

    //printf("\n\n");
    //telemetry_container.setBMI088DatasetRaw(bmi088RawData);
    //telemetry_container.setBMI088DatasetConverted(bmi088ConvertedData);
    //telemetry_container.setBMP280DatasetRaw(bmp280RawData);
    //telemetry_container.setBMP280DatasetConverted(bmp280ConvertedData);
    //telemetry_container.setTimeData1(timeKeeperStartOfWorld.deltaTime_us());
    //telemetry_container.setTimeData2(timeKeeperLaunch.deltaTime_us());
    //telemetry_container.setTimeData3(timeKeeperLaunch.deltaTime_us());
    //telemetry_container.printRawLogBytes();
    //telemetry_container.printPackagedTelemetryData();

    //flash.write_next_usable_page_size(telemetry_container.getPackagedRawBytes());

    //-----------------------------------------------------------------------------------------------------
    // clang-format off
        enum State {
            INIT,
            CALIBRATION,
            DETECTING_LAUNCH,
            POWERED_FLIGHT,
            DETECTING_APOGEE,
            PARACHUTE_DEPLOYMENT,
            DETECTING_LANDING,
            RECOVERY,
            ABORT };
    // clang-format on

    State currentState = INIT;

    // variables for the switch case
    bmi088DatasetConverted bmi088Data;
    //bmp280DatasetConverted bmp280Data;
    TimeKeeper coast_timer;
    auto scopeTimer = TimeKeeper();
    auto time = scopeTimer.deltaTime_us();
    time = 0;

    while (1) {
        switch (currentState) {
            case INIT:
                // calibrate the sensors
                bmi088.init();
                bmp280.init();

                // set the initial settings
                buzzer.set_frequency_hz(2700);
                buzzer.set_volume_percentage(0);
                led_r.set_pwm(0);
                led_g.set_pwm(0);
                //servo_E.set_angle_milli_degrees(0);
                servo_D_Parachute.set_angle_milli_degrees(0);
                servo_C_Pitch.set_angle_milli_degrees(0);
                //servo_B.set_angle_milli_degrees(0);
                servo_A_Yaw.set_angle_milli_degrees(0);
#ifndef DEBUG
                //buzzer.play_blocking(drv::buzzer::Chime::Chirp, 10000, default_buzzer_volume);

                puts("Init Servo Test!");
                //orientation_control_system.init_servo_test();
                control_loop.init_servo_test();

                puts("You have 30 seconds to put the rocket in it's place before calibration!");
                // Give some time for the operator to get the rocket where it needs to be after it was powered on before calibrating
                buzzer.play_blocking(drv::buzzer::Chime::BeepSlow, 10000, default_buzzer_volume);
                puts("20...");
                buzzer.play_blocking(drv::buzzer::Chime::BeepMedium, 10000, default_buzzer_volume);
                puts("10...");
                buzzer.play_blocking(drv::buzzer::Chime::BeepFast, 10000, default_buzzer_volume);
#endif

                currentState = CALIBRATION;
                break;
            case CALIBRATION:
                // Rocket should be seated and settled now
                buzzer.set_volume_percentage(default_buzzer_volume);

                puts("Calibrating the sensors!");

                // calibrate the sensors
                bmi088.run_gyro_calibration();
                bmp280.calculate_baseline_pressure_and_altitude_cm();

                buzzer.stop();

                puts("Calibration complete! Attempting to detect liftoff`");

                currentState = DETECTING_LAUNCH;
                break;
            case DETECTING_LAUNCH:
                // TODO: Wait on new data interrupt
                // TODO: Dataset should include the time since last sample
                bmi088Data = bmi088.blocking_wait_for_new_accel_data(INTERRUPT_TIMEOUT_US);

                // Detect liftoff
                if (state_detector.check_liftoff_detected(bmi088Data) == true) {
                    timeKeeperLaunch.mark();
                    control_loop.start();
                    // this is the first gyro based orientation data
                    orientation_calculator.update(bmi088Data);
                    currentState = POWERED_FLIGHT;
                    scopeTimer.mark();
#ifdef DEBUG
                    puts("TODO: REMOVE ME BEFORE FLIGHT! Liftoff detected!");
#endif
                } else {
                    // Only want to use accelerometer data if not under thrust
                    orientation_calculator.update_gravity(bmi088Data);
                }
                break;
            case POWERED_FLIGHT:
                // TODO: If we're only gonna log at 25hz the accelerometer should change the output data rate to 25hz
                // For now it's fine to just only get a snapshot of the acceleration data

                // Wait on new data interrupt
                //scopeTimer.mark();
                //time = scopeTimer.deltaTime_us();
                //printf("t%llu\n", time);
                bmi088Data = bmi088.blocking_wait_for_new_gyro_data(INTERRUPT_TIMEOUT_US);

                //scopeTimer.mark();
                orientation_calculator.update(bmi088Data);
                // The control loop is responsible for knowing when to update
                control_loop.tryUpdate(bmi088Data);

                // TODO: check abort
                /*if (state_detector.abort_conditions_detected(bmi088Data)) {
                    currentState = ABORT;
                }*/

                // Check for motor burnout using the accelerometer data
                if (state_detector.check_burnout_detected(bmi088Data) == true) {
                    coast_timer.mark();
                    currentState = DETECTING_APOGEE;

#ifdef DEBUG
                    puts("TODO: REMOVE ME BEFORE FLIGHT! Burnout detected!");
#endif
                }

                break;
            case DETECTING_APOGEE:
                // Wait on new data interrupt
                bmi088Data = bmi088.blocking_wait_for_new_gyro_data(INTERRUPT_TIMEOUT_US);
                orientation_calculator.update(bmi088Data);
                // In case we falsely detected burnout continue to try and update the control loop
                control_loop.tryUpdate(bmi088Data);

                // Check for apogee using the barometer data
                if (state_detector.check_apogee_detected(bmp280.pressure_Pa_to_relative_altitude_cm(bmp280.convert_data(bmp280.get_data_raw()).pressure_Pa)) ==
                    true) {
                    //TODO: Log the apogee detection
                    currentState = PARACHUTE_DEPLOYMENT;
#ifdef DEBUG
                    puts("TODO: REMOVE ME BEFORE FLIGHT! Apogee detected!");
#endif
                }
                // Use a timer as a safety check
                else if (coast_timer.deltaTime_ms() > MAX_COAST_TIME_MS) {
                    currentState = PARACHUTE_DEPLOYMENT;
#ifdef DEBUG
                    puts("TODO: REMOVE ME BEFORE FLIGHT! Max coast time reached!");
#endif
                }
                break;
            case PARACHUTE_DEPLOYMENT:
                // Deploy the parachute!
                parachute.deploy();
                puts("\n\n\n\n\n\n popped the chute!!!!\n");
                //TODO: Log the parachute deployment
                currentState = DETECTING_LANDING;
#ifdef DEBUG
                puts("TODO: REMOVE ME BEFORE FLIGHT! Parachute deployed!");
#endif
                break;
            case DETECTING_LANDING:
                // Wait on new data interrupt
                bmi088Data = bmi088.blocking_wait_for_new_gyro_data(INTERRUPT_TIMEOUT_US);
                orientation_calculator.update(bmi088Data);
                //for logging
                control_loop.tryUpdate(bmi088Data);

                // TODO: if it's been 100ms turn off the servos
                if (state_detector.check_landing_detected(bmi088Data) == true) {
                    currentState = RECOVERY;
#ifdef DEBUG
                    puts("TODO: REMOVE ME BEFORE FLIGHT! Landing detected!");
#endif
                }
                // TODO: make a 10 second timeout for landing detection
                break;
            case RECOVERY:
                // Log the last of the data
                logger.update(telemetry_container);
                logger.write_log_to_flash();
                // Play a tune to let the operator know the flight is over
                buzzer.play_blocking(drv::buzzer::Chime::Chirp, 1000 * 60 * 60 * 24, default_buzzer_volume);
                break;
            case ABORT:
                control_loop.abort();
                // If we're aborting pop the chute!
                currentState = PARACHUTE_DEPLOYMENT;
                break;
        }
        if (control_loop.did_update) {
            //update everything
            telemetry_container.setStartOfTheWorld(timeKeeperStartOfWorld.deltaTime_us());
            telemetry_container.setBMI088DatasetConverted(bmi088Data);
            telemetry_container.setBMP280DatasetConverted(bmp280.convert_data(bmp280.get_data_raw()));
            telemetry_container.setRelativeAltitude_cm(bmp280.pressure_Pa_to_relative_altitude_cm(bmp280.convert_data(bmp280.get_data_raw()).pressure_Pa));
            telemetry_container.setStateFlags(state_detector.get_state_flags());

            telemetry_container.setControlLoopData(control_loop.getControlLoopData());
            telemetry_container.setOrientation(orientation_calculator.getQuaternionValues());
            logger.update(telemetry_container);
        }
    }
}
