/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "../inc/drv_bmp280.h"
#include <stdio.h>

// Implementation stripped almost exactly from the the pico examples
namespace drv
{
    bmp280::bmp280(uint sclk, uint miso, uint mosi, uint cs, spi_module_num spi_module)
        : _sclk(sclk), _miso(miso), _mosi(mosi), _cs(cs)
    {
        if (spi_module == spi_module_0)
        {
            _spi_inst = spi0;
        }
        else if (spi_module == spi_module_1)
        {
            _spi_inst = spi1;
        }
        // 0.5MHz
        spi_init(_spi_inst, 500 * 1000);
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
        // See if SPI is working - interrograte the device for its I2C ID number, should be 0x60
        uint8_t id;
        read_registers(0xD0, &id, 1);
        printf("Chip ID is 0x%x\n", id);
        return id;
    }

    void bmp280::init()
    {
        // TODO: better error handling.
        if (readID() != 0x58)
        {
            puts("Error reading BMP id!");
        }
        read_compensation_parameters();

        write_register(0xF2, 0x1);  // Humidity oversampling register - going for x1
        write_register(0xF4, 0x27); // Set rest of oversampling modes and run mode to normal
    }

    void bmp280::getData()
    {
        int32_t humidity, pressure, temperature;
        bmp280_read_raw(&humidity, &pressure, &temperature);

        // These are the raw numbers from the chip, so we need to run through the
        // compensations to get human understandable numbers
        pressure = compensate_pressure(pressure);
        temperature = compensate_temp(temperature);
        humidity = compensate_humidity(humidity);

        // printf("Humidity = %.2f%%\n", humidity / 1024.0);
        // printf("Pressure = %dPa\n", pressure);
        // printf("Temp. = %.2fC\n", temperature / 100.0);
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
        sleep_ms(10);
    }

    void bmp280::read_registers(uint8_t reg, uint8_t *buf, uint16_t len)
    {
#define READ_BIT 0x80
        // For this particular device, we send the device the register we want to read
        // first, then subsequently read from the device. The register is auto incrementing
        // so we don't need to keep sending the register we want, just the first.
        reg |= READ_BIT;
        cs_select();
        spi_write_blocking(_spi_inst, &reg, 1);
        sleep_ms(10);
        spi_read_blocking(_spi_inst, 0, buf, len);
        cs_deselect();
        sleep_ms(10);
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

    uint32_t bmp280::compensate_humidity(int32_t adc_H)
    {
        int32_t v_x1_u32r;
        v_x1_u32r = (t_fine - ((int32_t)76800));
        v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1_u32r)) +
                       ((int32_t)16384)) >>
                      15) *
                     (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) * (((v_x1_u32r *
                                                                       ((int32_t)dig_H3)) >>
                                                                      11) +
                                                                     ((int32_t)32768))) >>
                         10) +
                        ((int32_t)2097152)) *
                           ((int32_t)dig_H2) +
                       8192) >>
                      14));
        v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
        v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
        v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

        return (uint32_t)(v_x1_u32r >> 12);
    }

    /* This function reads the manufacturing assigned compensation parameters from the device */
    void bmp280::read_compensation_parameters()
    {
        uint8_t buffer[26];

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

        dig_H1 = buffer[25];

        read_registers(0xE1, buffer, 8);

        dig_H2 = buffer[0] | (buffer[1] << 8);
        dig_H3 = (int8_t)buffer[2];
        dig_H4 = buffer[3] << 4 | (buffer[4] & 0xf);
        dig_H5 = (buffer[5] >> 4) | (buffer[6] << 4);
        dig_H6 = (int8_t)buffer[7];
    }

    void bmp280::bmp280_read_raw(int32_t *humidity, int32_t *pressure, int32_t *temperature)
    {
        uint8_t buffer[8];

        read_registers(0xF7, buffer, 8);
        *pressure = ((uint32_t)buffer[0] << 12) | ((uint32_t)buffer[1] << 4) | (buffer[2] >> 4);
        *temperature = ((uint32_t)buffer[3] << 12) | ((uint32_t)buffer[4] << 4) | (buffer[5] >> 4);
        *humidity = (uint32_t)buffer[6] << 8 | buffer[7];
    }

    void bmp280::forever_test()
    {

        int32_t humidity, pressure, temperature;

        while (1)
        {
            bmp280_read_raw(&humidity, &pressure, &temperature);

            // These are the raw numbers from the chip, so we need to run through the
            // compensations to get human understandable numbers
            pressure = compensate_pressure(pressure);
            temperature = compensate_temp(temperature);
            humidity = compensate_humidity(humidity);

            printf("Humidity = %.2f%%\n", humidity / 1024.0);
            printf("Pressure = %dPa\n", pressure);
            printf("Temp. = %.2fC\n", temperature / 100.0);

            sleep_ms(1000);
        }
    }
} // namespace drv
