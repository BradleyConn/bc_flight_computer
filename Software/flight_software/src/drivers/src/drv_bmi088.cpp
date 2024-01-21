#define __STDC_FORMAT_MACROS
#include "../inc/drv_bmi088.h"
#include "../inc/time_keeper.h"
#include "drv_bmi088.h"
#include <inttypes.h>
#include <stdio.h>

// Implementation stripped almost exactly from the the pico examples
namespace drv
{

bool bmi088::new_gyro_data_ready = false;
bmi088::bmi088(uint sclk, uint miso, uint mosi, uint accel_cs, uint gyro_cs, spi_module_num spi_module, uint accel_int_pin, uint gyro_int_pin)
    : _sclk(sclk)
    , _miso(miso)
    , _mosi(mosi)
    , _accel_cs(accel_cs)
    , _gyro_cs(gyro_cs)
    , _accel_int_pin(accel_int_pin)
    , _gyro_int_pin(gyro_int_pin)
{
    if (spi_module == spi_module_0) {
        _spi_inst = spi0;
    } else if (spi_module == spi_module_1) {
        _spi_inst = spi1;
    }
    // 10mhz is max according to spec. In reality, 12.5 has been tested and works.
    // Stick with 10mhz for now.
    constexpr auto desired_clock_rate = 10 * 1000 * 1000;
    auto achieved_clock_rate = spi_init(_spi_inst, desired_clock_rate);
    printf("Desired spi clock rate: %d, achieved: %d\n", desired_clock_rate, achieved_clock_rate);
    gpio_set_function(_sclk, GPIO_FUNC_SPI);
    gpio_set_function(_miso, GPIO_FUNC_SPI);
    gpio_set_function(_mosi, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(_accel_cs);
    gpio_set_dir(_accel_cs, GPIO_OUT);
    gpio_put(_accel_cs, 1);
    gpio_init(_gyro_cs);
    gpio_set_dir(_gyro_cs, GPIO_OUT);
    gpio_put(_gyro_cs, 1);

    // Set the interrupt pins as inputs
    gpio_init(_accel_int_pin);
    gpio_set_dir(_accel_int_pin, GPIO_IN);
    gpio_pull_down(_accel_int_pin);
    gpio_init(_gyro_int_pin);
    gpio_set_dir(_gyro_int_pin, GPIO_IN);
    gpio_pull_down(_gyro_int_pin);
}

// TODO: Unset pins
bmi088::~bmi088()
{
    //unset interrupts
}

uint8_t bmi088::read_accel_id()
{
    uint8_t id;
    accel_read_registers(0x00, &id, 1);
    printf("Chip ID is 0x%X, expected is 0x1E\n", id);
    return id;
}

uint8_t bmi088::read_gyro_id()
{
    uint8_t id;
    gyro_read_registers(0x00, &id, 1);
    printf("Chip ID is 0x%X, expected is 0x0F\n", id);
    return id;
}

void bmi088::accel_print_reg(uint8_t reg)
{
    uint8_t reg_val = 0xa5;
    accel_read_registers(reg, &reg_val, 1);
    printf("Reg 0x%02X, Val = 0x%02X\n", reg, reg_val);
}

void bmi088::accel_print_reg_expected(uint8_t reg, uint8_t expected)
{
    uint8_t reg_val = 0xa5;
    accel_read_registers(reg, &reg_val, 1);
    printf("Reg 0x%02X, expected = 0x%02X, Val = 0x%02X\n", reg, expected, reg_val);
}

void bmi088::accel_register_dump()
{
    accel_print_reg_expected(0x7D, 0x00);
    accel_print_reg_expected(0x7C, 0x03);
    accel_print_reg_expected(0x6D, 0x00);
    accel_print_reg_expected(0x58, 0x00);
    accel_print_reg_expected(0x54, 0x00);
    accel_print_reg_expected(0x53, 0x00);
    accel_print_reg_expected(0x49, 0x10);
    accel_print_reg_expected(0x48, 0x02);
    accel_print_reg_expected(0x47, 0x02);
    accel_print_reg_expected(0x46, 0x00);
    accel_print_reg_expected(0x45, 0x80);
    accel_print_reg_expected(0x41, 0x01);
    accel_print_reg_expected(0x40, 0xA8);
    accel_print_reg_expected(0x03, 0x00);
    accel_print_reg_expected(0x02, 0x00);
    accel_print_reg_expected(0x00, 0x1E);
}

void bmi088::init()
{
    // After POR the gyro is in normal mode, and the accel is in suspend mode.
    // So start up the accel.
    sleep_ms(1);
    // Do a dummy read to set the accel to SPI mode.
    puts("Dummy read to trigger SPI mode - expect 0x00");
    read_accel_id();
    puts("Do a soft reset");
    accel_write_register(0x7E, 0xB6);
    sleep_ms(50);
    // Do a dummy read to set the accel to SPI mode.
    puts("Dummy read to trigger SPI mode - expect 0x00");
    read_accel_id();

    // TODO: better error handling.
    while (read_accel_id() != 0x1E) {
        puts("error!");
    }
    if (read_gyro_id() != 0x0F) {
        puts("error!");
    }

    //accel_register_dump();

    // Set 4 to reg 0x7D to turn on accelerometer
    accel_write_register(0x7D, 0x04);
    sleep_ms(50);

    // Now configure it
    // Set the ODR 1600 (625 us), no oversampling - 280hz bandwidth
    accel_write_register(0x40, 0xAC);
    // Set +-24g
    accel_write_register(0x41, 0x03);
    // Set interrupt pin 1 as output
    accel_write_register(0x53, 0x0A);
    // Set data ready interrupt to pin 1
    accel_write_register(0x58, 0x04);
    // Set to active mode
    accel_write_register(0x7C, 0x00);

    // Now the Gyro
    // Set the interrupt 3 pin to active high
    gyro_write_register(0x16, 0x01);
    // Set the interrupt 3 pin to new data
    gyro_write_register(0x18, 0x01);
    // Set the range to 2000 deg/s, 61 milli-degree/sec/LSB
    gyro_write_register(0x0F, 0x00);
    // Set the ODR to 2000hz (500us), and bandwidth to 532Hz
    gyro_write_register(0x10, 0x00);
    // Enable new data interrupt to be triggered on new data
    gyro_write_register(0x15, 0x80);

    // sleep to let the new registers kick in
    sleep_ms(50);

    //set the interrupts
    //accel_int_pin
    //gpio_set_irq_enabled_with_callback(_accel_int_pin, GPIO_IRQ_EDGE_RISE, true, &accel_interrupt_handler);
    //gyro_int_pin
    gpio_set_irq_enabled_with_callback(_gyro_int_pin, GPIO_IRQ_EDGE_RISE, true, &gyro_interrupt_handler);
}

bool bmi088::accel_check_interrupt_data_ready()
{
    return gpio_get(_accel_int_pin);
}

bool bmi088::accel_interrupt_reg_clear()
{
    uint8_t reg_val = 0x00;
    accel_read_registers(0x1D, &reg_val, 1);
    if (reg_val == 0x00) {
        return false;
    }
    return true;
}

bool bmi088::gyro_check_interrupt_data_ready()
{
    return new_gyro_data_ready;
}

bool bmi088::gyro_interrupt_reg_clear()
{
    if (new_gyro_data_ready == false) {
        return false;
    } else {
        new_gyro_data_ready = false;
        return true;
    }
}

void bmi088::gyro_interrupt_handler(uint gpio, uint32_t events)
{
    new_gyro_data_ready = true;
}

AccelDataRaw bmi088::accel_get_data_raw()
{
    uint8_t accel_data_u8[9];
    int16_t accel_data_i16[9];
    for (uint32_t i = 0; i < sizeof(accel_data_u8); i++) {
        accel_data_i16[i] = 0;
        accel_data_u8[i] = 0;
    }

    accel_read_registers(0x12, accel_data_u8, 9);

    for (uint32_t i = 0; i < sizeof(accel_data_u8); i++) {
        accel_data_i16[i] = accel_data_u8[i];
    }

    AccelDataRaw accelDataRaw;
    accelDataRaw.x = (int16_t)((accel_data_i16[1] << 8) + accel_data_i16[0]);
    accelDataRaw.y = (int16_t)((accel_data_i16[3] << 8) + accel_data_i16[2]);
    accelDataRaw.z = (int16_t)((accel_data_i16[5] << 8) + accel_data_i16[4]);
    accelDataRaw.time = (accel_data_u8[8] << 16) + (accel_data_u8[7] << 8) + accel_data_u8[6];

    return accelDataRaw;
}

AccelDataConverted bmi088::accel_convert_data(AccelDataRaw accel_data_raw)
{
    AccelDataConverted accelDataConverted;
    //pulled from datasheet - val_in_mg = raw_int16_t /32768 * 1000 * 2^(range+1) * 1.5
    constexpr auto range24g = 3;
    //Do some earlier evaluation (1000*1.5=1500) so no floats are needed
    //Shift instead of trying to constexpr a pow - 2^(range24g+1) = 1<<range24g+1
    constexpr int64_t raw_to_uG = 1500 * (1 << (range24g + 1));

    accelDataConverted.x_mg = (int64_t)(accel_data_raw.x * raw_to_uG) / 32768;
    accelDataConverted.y_mg = (int64_t)(accel_data_raw.y * raw_to_uG) / 32768;
    accelDataConverted.z_mg = (int64_t)(accel_data_raw.z * raw_to_uG) / 32768;
    // LSB is 39.0625 us
    accelDataConverted.time_us = ((uint64_t)accel_data_raw.time * (uint64_t)390625) / (uint64_t)10000;
    return accelDataConverted;
}

AccelTemperatureRaw bmi088::accel_get_temperature_raw()
{
    //temperature reg only updates every 1.28 secconds - maybe don't use it?
    uint8_t temperature_reg[2];
    uint16_t temperature_u11 = 0;
    int16_t temperature_i11 = 0;
    accel_read_registers(0x22, temperature_reg, 2);
    //pulled straight from the data sheet
    temperature_u11 = (temperature_reg[0] * 8) + (temperature_reg[1] / 32);
    if (temperature_u11 > 1023) {
        temperature_i11 = temperature_u11 - 2048;
    } else {
        temperature_i11 = temperature_u11;
    }
    AccelTemperatureRaw accelTemperatureRaw;
    accelTemperatureRaw.temperature = temperature_i11;
    return accelTemperatureRaw;
}
AccelTemperatureConverted bmi088::accel_convert_temperature(AccelTemperatureRaw accel_temperature_raw)
{
    AccelTemperatureConverted accelTemperatureConverted;
    accelTemperatureConverted.temperature_deg_mC = (accel_temperature_raw.temperature * 125) + 23 * 1000;
    return accelTemperatureConverted;
}

GyroDataRaw bmi088::gyro_get_data_raw()
{
    uint8_t gyro_data_u8[6];
    int16_t gyro_data_i16[6];
    for (uint32_t i = 0; i < sizeof(gyro_data_u8); i++) {
        gyro_data_i16[i] = 0;
        gyro_data_u8[i] = 0;
    }

    gyro_read_registers(0x02, gyro_data_u8, 6);

    for (uint32_t i = 0; i < sizeof(gyro_data_u8); i++) {
        gyro_data_i16[i] = gyro_data_u8[i];
    }

    GyroDataRaw gyroDataRaw;
    gyroDataRaw.x = (int16_t)((gyro_data_i16[1] << 8) + gyro_data_i16[0]);
    gyroDataRaw.y = (int16_t)((gyro_data_i16[3] << 8) + gyro_data_i16[2]);
    gyroDataRaw.z = (int16_t)((gyro_data_i16[5] << 8) + gyro_data_i16[4]);

    return gyroDataRaw;
}

GyroDataConverted bmi088::gyro_convert_data(GyroDataRaw gyro_data_raw)
{
    GyroDataConverted gyroDataConverted;
    //  rate of 2000 deg/sec is 61 milli-degrees/sec/LSB
    gyroDataConverted.x_milli_degrees_per_sec = (gyro_data_raw.x * 610)/10;
    gyroDataConverted.y_milli_degrees_per_sec = (gyro_data_raw.y * 610)/10;
    gyroDataConverted.z_milli_degrees_per_sec = (gyro_data_raw.z * 610)/10;
    return gyroDataConverted;
}

bmi088DatasetRaw bmi088::get_data_raw()
{
    bmi088DatasetRaw bmi088DatasetRaw;
    bmi088DatasetRaw.accel_data_raw = accel_get_data_raw();
    bmi088DatasetRaw.accel_temperature_raw = accel_get_temperature_raw();
    bmi088DatasetRaw.gyro_data_raw = gyro_get_data_raw();
    return bmi088DatasetRaw;
}

bmi088DatasetConverted bmi088::convert_data(bmi088DatasetRaw bmi088DatasetRaw)
{
    bmi088DatasetConverted bmi088DatasetConverted;
    bmi088DatasetConverted.accel_data_converted = accel_convert_data(bmi088DatasetRaw.accel_data_raw);
    bmi088DatasetConverted.accel_temperature_converted = accel_convert_temperature(bmi088DatasetRaw.accel_temperature_raw);
    bmi088DatasetConverted.gyro_data_converted = gyro_convert_data(bmi088DatasetRaw.gyro_data_raw);
    return bmi088DatasetConverted;
}

void bmi088::print_data_raw(bmi088DatasetRaw bmi088_dataset_raw)
{
    printf("Accel X = %d\n", bmi088_dataset_raw.accel_data_raw.x);
    printf("Accel Y = %d\n", bmi088_dataset_raw.accel_data_raw.y);
    printf("Accel Z = %d\n", bmi088_dataset_raw.accel_data_raw.z);
    printf("Accel Time = %lu\n", bmi088_dataset_raw.accel_data_raw.time);
    printf("Accel Temp = %d\n", bmi088_dataset_raw.accel_temperature_raw.temperature);
    printf("Gyro X = %ld\n", bmi088_dataset_raw.gyro_data_raw.x);
    printf("Gyro Y = %ld\n", bmi088_dataset_raw.gyro_data_raw.y);
    printf("Gyro Z = %ld\n", bmi088_dataset_raw.gyro_data_raw.z);
}

void bmi088::print_data_converted(bmi088DatasetConverted bmi088_dataset_converted)
{
    printf("Accel X = %ld mg\n", bmi088_dataset_converted.accel_data_converted.x_mg);
    printf("Accel Y = %ld mg\n", bmi088_dataset_converted.accel_data_converted.y_mg);
    printf("Accel Z = %ld mg\n", bmi088_dataset_converted.accel_data_converted.z_mg);
    printf("Accel Time = %lu us\n", bmi088_dataset_converted.accel_data_converted.time_us);
    printf("Accel Temp = %ld deg mC\n", bmi088_dataset_converted.accel_temperature_converted.temperature_deg_mC);
    printf("Gyro X = %ld milli-degrees/sec\n", bmi088_dataset_converted.gyro_data_converted.x_milli_degrees_per_sec);
    printf("Gyro Y = %ld milli-degrees/sec\n", bmi088_dataset_converted.gyro_data_converted.y_milli_degrees_per_sec);
    printf("Gyro Z = %ld milli-degrees/sec\n", bmi088_dataset_converted.gyro_data_converted.z_milli_degrees_per_sec);
}

void bmi088::print_data_converted_floats(bmi088DatasetConverted bmi088_dataset_converted)
{
    printf("Accel X = %.2f g\n", static_cast<float>(bmi088_dataset_converted.accel_data_converted.x_mg) / 1000.0);
    printf("Accel Y = %.2f g\n", static_cast<float>(bmi088_dataset_converted.accel_data_converted.y_mg) / 1000.0);
    printf("Accel Z = %.2f g\n", static_cast<float>(bmi088_dataset_converted.accel_data_converted.z_mg) / 1000.0);
    printf("Accel Time = %.2f ms\n", static_cast<float>(bmi088_dataset_converted.accel_data_converted.time_us) / 1000.0);
    printf("Accel Temp = %.2f deg C\n", static_cast<float>(bmi088_dataset_converted.accel_temperature_converted.temperature_deg_mC) / 1000.0);
    printf("Gyro X = %.2f deg/s\n", static_cast<float>(bmi088_dataset_converted.gyro_data_converted.x_milli_degrees_per_sec) / 1000.0);
    printf("Gyro Y = %.2f deg/s\n", static_cast<float>(bmi088_dataset_converted.gyro_data_converted.y_milli_degrees_per_sec) / 1000.0);
    printf("Gyro Z = %.2f deg/s\n", static_cast<float>(bmi088_dataset_converted.gyro_data_converted.z_milli_degrees_per_sec) / 1000.0);
}

inline void bmi088::accel_cs_select()
{
    asm volatile("nop \n nop \n nop");
    gpio_put(_accel_cs, 0); // Active low
    asm volatile("nop \n nop \n nop");
}

inline void bmi088::accel_cs_deselect()
{
    asm volatile("nop \n nop \n nop");
    gpio_put(_accel_cs, 1);
    asm volatile("nop \n nop \n nop");
}

inline void bmi088::gyro_cs_select()
{
    asm volatile("nop \n nop \n nop");
    gpio_put(_gyro_cs, 0); // Active low
    asm volatile("nop \n nop \n nop");
}

inline void bmi088::gyro_cs_deselect()
{
    asm volatile("nop \n nop \n nop");
    gpio_put(_gyro_cs, 1);
    asm volatile("nop \n nop \n nop");
}

void bmi088::accel_write_register(uint8_t reg, uint8_t data)
{
    write_register(reg, data, cs_type::accel);
}

void bmi088::gyro_write_register(uint8_t reg, uint8_t data)
{
    write_register(reg, data, gyro);
}

void bmi088::write_register(uint8_t reg, uint8_t data, cs_type cs)
{
    uint8_t buf[2];
    buf[0] = reg & 0x7f; // remove read bit as this is a write
    buf[1] = data;
    if (cs == accel) {
        accel_cs_select();
    } else if (cs == gyro) {
        gyro_cs_select();
    }
    spi_write_blocking(_spi_inst, buf, 2);
    accel_cs_deselect();
    gyro_cs_deselect();
}

#define READ_BIT 0x80
void bmi088::accel_read_registers(uint8_t reg, uint8_t* buf, uint16_t len)
{
    // The accelerometer needs a dummy read first unlike the gyro :facepalm:
    reg |= READ_BIT;
    accel_cs_select();
    spi_write_blocking(_spi_inst, &reg, 1);
    spi_read_blocking(_spi_inst, 0, buf, 1);
    spi_read_blocking(_spi_inst, 0, buf, len);
    accel_cs_deselect();
}
void bmi088::gyro_read_registers(uint8_t reg, uint8_t* buf, uint16_t len)
{
    read_registers(reg, buf, len, gyro);
}
void bmi088::read_registers(uint8_t reg, uint8_t* buf, uint16_t len, cs_type cs)
{
    // For this particular device, we send the device the register we want to read
    // first, then subsequently read from the device. The register is auto incrementing
    // so we don't need to keep sending the register we want, just the first.
    reg |= READ_BIT;
    if (cs == accel) {
        accel_cs_select();
    } else if (cs == gyro) {
        gyro_cs_select();
    }
    spi_write_blocking(_spi_inst, &reg, 1);
    spi_read_blocking(_spi_inst, 0, buf, len);
    accel_cs_deselect();
    gyro_cs_deselect();
}

} // namespace drv
