#ifndef _DRV_BMP280_H
#define _DRV_BMP280_H

#include "hardware/spi.h"
#include "pico/stdlib.h"


struct bmp280DatasetRaw {
    int32_t temperature_raw;
    int32_t pressure_raw;
};

struct bmp280DatasetConverted {
    int32_t temperature_deg_cC; // centi degrees C
    uint32_t pressure_Pa;
};

namespace drv
{

class bmp280
{
public:
    enum spi_module_num {
        spi_module_0 = 0x00,
        spi_module_1 = 0x01,
    };

    bmp280(uint sclk, uint miso, uint mosi, uint cs, spi_module_num spi_module);
    ~bmp280();
    uint8_t readID();
    void init();
    bmp280DatasetRaw get_data_raw();
    bmp280DatasetConverted convert_data(bmp280DatasetRaw data);
    void forever_test();
    void print_data_raw(bmp280DatasetRaw data);
    void print_data_converted(bmp280DatasetConverted data);

private:
    // Taken from pico examples
    inline void cs_select();
    inline void cs_deselect();
    void write_register(uint8_t reg, uint8_t data);
    void read_registers(uint8_t reg, uint8_t* buf, uint16_t len);
    int32_t compensate_temp(int32_t adc_T);
    uint32_t compensate_pressure(int32_t adc_P);
    void read_compensation_parameters();

private:
    uint _sclk;
    uint _miso;
    uint _mosi;
    uint _cs;
    spi_inst_t* _spi_inst;

private: // temporary variables the example code used.
    int32_t t_fine;

    uint16_t dig_T1;
    int16_t dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;

}; // class bmp280

}; // namespace drv
#endif
