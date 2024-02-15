/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "../inc/drv_bmp280.h"
#include "drv_bmp280.h"
#include <math.h>
#include <stdio.h>

// Implementation stripped almost exactly from the the pico examples
namespace drv
{

bmp280::bmp280(uint sclk, uint miso, uint mosi, uint cs, spi_module_num spi_module) : _sclk(sclk), _miso(miso), _mosi(mosi), _cs(cs)
{
    if (spi_module == spi_module_0) {
        _spi_inst = spi0;
    } else if (spi_module == spi_module_1) {
        _spi_inst = spi1;
    }
    // 10Mhz
    const auto desired_clockrate = 10000 * 1000;
    auto clockrate = spi_init(_spi_inst, desired_clockrate);
    printf("BMP280 desired spi clock: %d Hz, actual: %d Hz\n", desired_clockrate, clockrate);
    gpio_set_function(_sclk, GPIO_FUNC_SPI);
    gpio_set_function(_miso, GPIO_FUNC_SPI);
    gpio_set_function(_mosi, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(_cs);
    gpio_set_dir(_cs, GPIO_OUT);
    gpio_put(_cs, 1);
}

// TODO: Unset pins
bmp280::~bmp280()
{
}

uint8_t bmp280::readID()
{
    // See if SPI is working - interrograte the device for its I2C ID number, should be 0x58
    uint8_t id;
    read_registers(0xD0, &id, 1);
    printf("Chip ID is 0x%x\n", id);
    return id;
}

void bmp280::init()
{
    // TODO: better error handling.
    if (readID() != 0x58) {
        puts("Error reading BMP id!");
    }
    read_compensation_parameters();

    // The settings in the following 2 registers give a sample rate of about 26-27hz or a period of 37-38ms
    // 010 - temp oversample x2
    // 101 - pressure oversample x16
    // 11 - normal mode
    write_register(0xF4, 0x57);
    write_register(0xF5, 0x00); // Set standby time to 0.5ms (smallest) and IIR filter to 0, and 4-wire SPI mode
    //Let the registers take
    sleep_ms(100);
}

void bmp280::calculate_baseline_pressure_and_altitude_cm()
{
    int64_t base_pressure = 0;
    for (int i = 0; i < 50; i++) {
        bmp280DatasetRaw data = get_data_raw();
        bmp280DatasetConverted converted_data = convert_data(data);
        base_pressure += converted_data.pressure_Pa;
        sleep_ms(50);
    }
    base_pressure /= 50;
    _baseline_pressure_Pa = base_pressure;
    _baseline_altitude_cm = pressure_Pa_to_absolute_altitude_cm(_baseline_pressure_Pa);
}

bmp280DatasetRaw bmp280::get_data_raw()
{
    bmp280DatasetRaw data;
    uint8_t buffer[6];

    read_registers(0xF7, buffer, 6);
    data.pressure_raw = ((uint32_t)buffer[0] << 12) | ((uint32_t)buffer[1] << 4) | (buffer[2] >> 4);
    data.temperature_raw = ((uint32_t)buffer[3] << 12) | ((uint32_t)buffer[4] << 4) | (buffer[5] >> 4);
    return data;
}

bmp280DatasetConverted bmp280::convert_data(bmp280DatasetRaw data)
{
    bmp280DatasetConverted converted_data;
    converted_data.temperature_deg_cC = compensate_temp(data.temperature_raw);
    converted_data.pressure_Pa = compensate_pressure(data.pressure_raw);
    return converted_data;
}

inline void bmp280::cs_select()
{
    asm volatile("nop \n nop \n nop");
    gpio_put(_cs, 0); // Active low
    asm volatile("nop \n nop \n nop");
}

inline void bmp280::cs_deselect()
{
    asm volatile("nop \n nop \n nop");
    gpio_put(_cs, 1);
    asm volatile("nop \n nop \n nop");
}

void bmp280::write_register(uint8_t reg, uint8_t data)
{
    uint8_t buf[2];
    buf[0] = reg & 0x7f; // remove read bit as this is a write
    buf[1] = data;
    cs_select();
    spi_write_blocking(_spi_inst, buf, 2);
    cs_deselect();
}

void bmp280::read_registers(uint8_t reg, uint8_t* buf, uint16_t len)
{
#define READ_BIT 0x80
    // For this particular device, we send the device the register we want to read
    // first, then subsequently read from the device. The register is auto incrementing
    // so we don't need to keep sending the register we want, just the first.
    reg |= READ_BIT;
    cs_select();
    spi_write_blocking(_spi_inst, &reg, 1);
    spi_read_blocking(_spi_inst, 0, buf, len);
    cs_deselect();
}

/* The following compensation functions are required to convert from the raw ADC
data from the chip to something usable. Each chip has a different set of
compensation parameters stored on the chip at point of manufacture, which are
read from the chip at startup and used in these routines.
*/
int32_t bmp280::compensate_temp(int32_t adc_T)
{
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;

    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

uint32_t bmp280::compensate_pressure(int32_t adc_P)
{
    int32_t var1, var2;
    uint32_t p;
    var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)dig_P6);
    var2 = var2 + ((var1 * ((int32_t)dig_P5)) << 1);
    var2 = (var2 >> 2) + (((int32_t)dig_P4) << 16);
    var1 = (((dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)dig_P2) * var1) >> 1)) >> 18;
    var1 = ((((32768 + var1)) * ((int32_t)dig_P1)) >> 15);
    if (var1 == 0)
        return 0;

    p = (((uint32_t)(((int32_t)1048576) - adc_P) - (var2 >> 12))) * 3125;
    if (p < 0x80000000)
        p = (p << 1) / ((uint32_t)var1);
    else
        p = (p / (uint32_t)var1) * 2;

    var1 = (((int32_t)dig_P9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
    var2 = (((int32_t)(p >> 2)) * ((int32_t)dig_P8)) >> 13;
    p = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));

    return p;
}

/* This function reads the manufacturing assigned compensation parameters from
 * the device */
void bmp280::read_compensation_parameters()
{
    uint8_t buffer[24];

    read_registers(0x88, buffer, 24);

    dig_T1 = buffer[0] | (buffer[1] << 8);
    dig_T2 = buffer[2] | (buffer[3] << 8);
    dig_T3 = buffer[4] | (buffer[5] << 8);

    dig_P1 = buffer[6] | (buffer[7] << 8);
    dig_P2 = buffer[8] | (buffer[9] << 8);
    dig_P3 = buffer[10] | (buffer[11] << 8);
    dig_P4 = buffer[12] | (buffer[13] << 8);
    dig_P5 = buffer[14] | (buffer[15] << 8);
    dig_P6 = buffer[16] | (buffer[17] << 8);
    dig_P7 = buffer[18] | (buffer[19] << 8);
    dig_P8 = buffer[20] | (buffer[21] << 8);
    dig_P9 = buffer[22] | (buffer[23] << 8);
}

int32_t bmp280::get_baseline_pressure_Pa()
{
    return _baseline_pressure_Pa;
}

int32_t bmp280::get_baseline_altitude_cm()
{
    return _baseline_altitude_cm;
}

int32_t bmp280::pressure_Pa_to_relative_altitude_cm(int32_t pressure_Pa)
{
    return pressure_Pa_to_absolute_altitude_cm(pressure_Pa) - _baseline_altitude_cm;
}

int32_t bmp280::pressure_Pa_to_absolute_altitude_cm(int32_t pressure_Pa)
{
    // This is a very rough estimate (as seen on wiki, weather.gov, adafruit, etc.)
    // and should be calibrated before flight and probably take into account temperature if accurate altitute is needed.
    // XXX: This is using floats and powf so it'll be slow.
    // This formula is for meters, and pa so divide by 100 to get hPa, so 1013.25 becomes 101325, add .0 to signify float
    float altitude_m = 44330.7694 * (1 - powf(pressure_Pa / 101325.0, 0.190284));
    return altitude_m * 100;
}

float bmp280::cm_to_feet(int32_t altitude_cm)
{
    return altitude_cm * .0328084;
}

void bmp280::forever_test()
{
    while (1) {
        bmp280DatasetRaw data = get_data_raw();

        // These are the raw numbers from the chip, so we need to run through the
        // compensations to get human understandable numbers
        bmp280DatasetConverted converted_data = convert_data(data);
        printf("Pressure = %ld Pa\n", converted_data.pressure_Pa);
        // XXX: Uses floats. RP2040 has no fp hardware. Software fp will be super slow. Do not use in flight code.
        printf("Temp. = %ld C\n", converted_data.temperature_deg_cC / 100);

        // Estimate the altitude using the pressure.
        // This is a very rough estimate (as seen on wiki, weather.gov, adafruit, etc.)
        // and should be calibrated before flight and probably take into account temperature if accurate altitute is needed.
        // XXX: This is using floats and powf so it'll be slow.
        // Don't include it in flight code so don't premote it with helper functions or anything.
        // This formula is for meters, and pa so divide by 100 to get hPa, so 1013.25 becomes 101325, add .0 to signify float
        float altitude = 443307.694 * (1 - powf(converted_data.pressure_Pa / 101325.0, 0.190284));
        printf("Altitude = %.2fm or %.fft\n", altitude, altitude * 3.28084);

        sleep_ms(1000);
    }
}

void bmp280::print_data_raw(bmp280DatasetRaw data)
{
    printf("Pressure raw = %ld\n", data.pressure_raw);
    printf("Temperature raw = %ld\n", data.temperature_raw);
}

void bmp280::print_data_converted(bmp280DatasetConverted data)
{
    printf("Pressure = %ld Pa\n", data.pressure_Pa);
    printf("Temp. = %ld centi C\n", data.temperature_deg_cC);
}

} // namespace drv
