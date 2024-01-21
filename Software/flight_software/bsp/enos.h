/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// -----------------------------------------------------
// NOTE: THIS HEADER IS ALSO INCLUDED BY ASSEMBLER SO
//       SHOULD ONLY CONSIST OF PREPROCESSOR DIRECTIVES
// -----------------------------------------------------

#ifndef _BOARDS_ENOS_H
#define _BOARDS_ENOS_H

// For board detection
#define ENOS

// On some samples, the xosc can take longer to stabilize than is usual
#ifndef PICO_XOSC_STARTUP_DELAY_MULTIPLIER
#define PICO_XOSC_STARTUP_DELAY_MULTIPLIER 64
#endif

//------------- UART -------------//
//CONFLICTS WITH I2C
#ifndef PICO_DEFAULT_UART
#define PICO_DEFAULT_UART 1
#endif

#ifndef PICO_DEFAULT_UART_TX_PIN
#define PICO_DEFAULT_UART_TX_PIN 20
#endif

//XXX: MADE A PIN ASSIGNMENT MISTAKE
//NO UART ON PIN 23
//#ifndef PICO_DEFAULT_UART_RX_PIN
//#define PICO_DEFAULT_UART_RX_PIN 23
//#endif

//------------- LED -------------//
#ifndef PICO_DEFAULT_LED_PIN_RED
#define PICO_DEFAULT_LED_PIN_RED 13
#endif

#ifndef PICO_DEFAULT_LED_PIN_GREEN
#define PICO_DEFAULT_LED_PIN_GREEN 15
#endif

//------------ BUZZER -----------//
#ifndef PICO_DEFAULT_BUZZER_PIN
#define PICO_DEFAULT_BUZZER_PIN 7
#endif

//------------ SERVOS -----------//
#ifndef PICO_DEFAULT_SERVO_A_PIN
#define PICO_DEFAULT_SERVO_A_PIN 16
#endif

#ifndef PICO_DEFAULT_SERVO_B_PIN
#define PICO_DEFAULT_SERVO_B_PIN 18
#endif

#ifndef PICO_DEFAULT_SERVO_C_PIN
#define PICO_DEFAULT_SERVO_C_PIN 21
#endif

#ifndef PICO_DEFAULT_SERVO_D_PIN
#define PICO_DEFAULT_SERVO_D_PIN 22
#endif

#ifndef PICO_DEFAULT_SERVO_E_PIN
#define PICO_DEFAULT_SERVO_E_PIN 24
#endif

//------------- I2C -------------//
// CONFLICTS WITH UART
#ifndef PICO_DEFAULT_I2C
#define PICO_DEFAULT_I2C 0
#endif

#ifndef PICO_DEFAULT_I2C_SDA_PIN
#define PICO_DEFAULT_I2C_SDA_PIN 20
#endif

#ifndef PICO_DEFAULT_I2C_SCL_PIN
#define PICO_DEFAULT_I2C_SCL_PIN 23
#endif

//------------- SPI BMI088 -------------//
#ifndef PICO_DEFAULT_SPI_BMI088
#define PICO_DEFAULT_SPI_BMI088 0
#endif

#ifndef PICO_DEFAULT_SPI_MOSI_PIN_BMI088
#define PICO_DEFAULT_SPI_MOSI_PIN_BMI088 3
#endif

#ifndef PICO_DEFAULT_SPI_MISO_PIN_BMI088
#define PICO_DEFAULT_SPI_MISO_PIN_BMI088 0
#endif

#ifndef PICO_DEFAULT_SPI_SCLK_PIN_BMI088
#define PICO_DEFAULT_SPI_SCLK_PIN_BMI088 2
#endif

#ifndef PICO_DEFAULT_SPI_ACCEL_CS_PIN_BMI088
#define PICO_DEFAULT_SPI_ACCEL_CS_PIN_BMI088 1
#endif

#ifndef PICO_DEFAULT_SPI_GYRO_CS_PIN_BMI088
#define PICO_DEFAULT_SPI_GYRO_CS_PIN_BMI088 5
#endif

#ifndef PICO_DEFAULT_SPI_ACCEL_INT_PIN_BMI088
#define PICO_DEFAULT_SPI_ACCEL_INT_PIN_BMI088 6
#endif

#ifndef PICO_DEFAULT_SPI_GYRO_INT_PIN_BMI088
#define PICO_DEFAULT_SPI_GYRO_INT_PIN_BMI088 4
#endif

//------------- SPI BMP280 -------------//
#ifndef PICO_DEFAULT_SPI_BMP280
#define PICO_DEFAULT_SPI_BMP280 1
#endif

#ifndef PICO_DEFAULT_SPI_MOSI_PIN_BMP280
#define PICO_DEFAULT_SPI_MOSI_PIN_BMP280 11
#endif

#ifndef PICO_DEFAULT_SPI_MISO_PIN_BMP280
#define PICO_DEFAULT_SPI_MISO_PIN_BMP280 8
#endif

#ifndef PICO_DEFAULT_SPI_SCLK_PIN_BMP280
#define PICO_DEFAULT_SPI_SCLK_PIN_BMP280 10
#endif

#ifndef PICO_DEFAULT_SPI_CS_PIN_BMP280
#define PICO_DEFAULT_SPI_CS_PIN_BMP280 9
#endif

//------------- FLASH -------------//

#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1

#ifndef PICO_FLASH_SPI_CLKDIV
#define PICO_FLASH_SPI_CLKDIV 2
#endif

#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)
#endif

// Drive high to force power supply into PWM mode (lower ripple on 3V3 at light loads)
#define PICO_SMPS_MODE_PIN 23

#ifndef PICO_RP2040_B0_SUPPORTED
#define PICO_RP2040_B0_SUPPORTED 1
#endif

#endif
