cmake_minimum_required(VERSION 3.13)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(../pico-sdk/pico_sdk_init.cmake)
include(pico_extras_import_optional.cmake)


project(my_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/bsp)
set(PICO_BOARD enos)
set(CMAKE_BUILD_TYPE Debug)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()


# rest of your project
add_executable(main
    src/main.cpp

    src/drivers/src/drv_servo.cpp
    src/drivers/src/drv_led.cpp
    src/drivers/src/drv_bmp280.cpp
    src/drivers/src/drv_bmi088.cpp
    src/drivers/src/drv_buzzer.cpp
    src/drivers/src/drv_flash.cpp
    src/drivers/src/drv_gpio.cpp

    src/system/src/torque_to_tvc_angle.cpp
    src/system/src/time_keeper.cpp
    src/system/src/telemetry_container.cpp
    src/system/src/thrust_curves/thrust_curve_E12.cpp
    src/system/src/thrust_curves/thrust_curve_E9.cpp

    src/tests/characterize_servo_test.cpp
    src/tests/orientation_test.cpp
    src/tests/chute_ejection_test.cpp

    libs/Orientation/Quaternion/Quaternion.cpp
    libs/Orientation/Orientation/Orientation.cpp

)
target_include_directories(main
 PRIVATE
  src/drivers/inc/
  src/system/inc/
  )
add_compile_options(-Wall
        -Wextra
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

# turn on more warnings
target_compile_options(main PRIVATE -Wall)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(main pico_stdlib hardware_pwm hardware_spi hardware_flash)

pico_enable_stdio_usb(main 1)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(main)
