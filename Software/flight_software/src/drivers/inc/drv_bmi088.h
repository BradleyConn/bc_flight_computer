#ifndef _DRV_BM088_H
#define _DRV_BM088_H

#include "hardware/spi.h"
#include "pico/stdlib.h"

struct AccelDataRaw {
    int16_t x;
    int16_t y;
    int16_t z;
    uint32_t time;
};
struct AccelTemperatureRaw {
    int16_t temperature;
};
struct GyroDataRaw {
    int32_t x;
    int32_t y;
    int32_t z;
};
struct bmi088DatasetRaw {
    AccelDataRaw accel_data_raw;
    AccelTemperatureRaw accel_temperature_raw;
    GyroDataRaw gyro_data_raw;
};

struct AccelDataConverted {
    int32_t x_mg;
    int32_t y_mg;
    int32_t z_mg;
    uint32_t time_us;
};
struct AccelTemperatureConverted {
    int32_t temperature_deg_mC;
};
struct GyroDataConverted {
    int32_t x_milli_degrees_per_sec;
    int32_t y_milli_degrees_per_sec;
    int32_t z_milli_degrees_per_sec;
};
struct bmi088DatasetConverted {
    AccelDataConverted accel_data_converted;
    AccelTemperatureConverted accel_temperature_converted;
    GyroDataConverted gyro_data_converted;
};

namespace drv
{
class bmi088
{
public:
    enum spi_module_num {
        spi_module_0 = 0x00,
        spi_module_1 = 0x01,
    };
    enum cs_type {
        accel = 0x00,
        gyro = 0x01,
    };

    bmi088(uint sclk, uint miso, uint mosi, uint accel_cs, uint gyro_cs, spi_module_num spi_module);
    ~bmi088();
    uint8_t read_gyro_id();
    uint8_t read_accel_id();
    void init();
    AccelDataRaw accel_get_data_raw();
    AccelTemperatureRaw accel_get_temperature_raw();
    GyroDataRaw gyro_get_data_raw();
    bmi088DatasetRaw get_data_raw();
    AccelDataConverted accel_convert_data(AccelDataRaw accel_data_raw);
    AccelTemperatureConverted accel_convert_temperature(AccelTemperatureRaw accel_temperature_raw);
    GyroDataConverted gyro_convert_data(GyroDataRaw gyro_data_raw);
    bmi088DatasetConverted convert_data(bmi088DatasetRaw bmi088_dataset_raw);
    void print_data_raw(bmi088DatasetRaw bmi088_dataset_raw);
    void print_data_converted(bmi088DatasetConverted bmi088_dataset_converted);
    void print_data_converted_floats(bmi088DatasetConverted bmi088_dataset_converted);

private:
    // Taken from pico examples
    inline void accel_cs_select();
    inline void accel_cs_deselect();
    inline void gyro_cs_select();
    inline void gyro_cs_deselect();
    void accel_write_register(uint8_t reg, uint8_t data);
    void accel_read_registers(uint8_t reg, uint8_t* buf, uint16_t len);
    void gyro_write_register(uint8_t reg, uint8_t data);
    void gyro_read_registers(uint8_t reg, uint8_t* buf, uint16_t len);
    void write_register(uint8_t reg, uint8_t data, cs_type cs);
    void read_registers(uint8_t reg, uint8_t* buf, uint16_t len, cs_type cs);
    void accel_print_reg(uint8_t reg);
    void accel_print_reg_expected(uint8_t reg, uint8_t expected);
    void accel_register_dump();

private:
    uint _sclk;
    uint _miso;
    uint _mosi;
    uint _accel_cs;
    uint _gyro_cs;
    spi_inst_t* _spi_inst;
}; // class bmi088
}; // namespace drv
#endif
