#ifndef _DRV_BM088_H
#define _DRV_BM088_H

#include "hardware/spi.h"
#include "pico/stdlib.h"

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

    bmi088(uint sclk, uint miso, uint mosi, uint accel_cs, uint gyro_cs,
           spi_module_num spi_module);
    ~bmi088();
    uint8_t readGyroID();
    uint8_t readAccelID();
    void init();
    void getData();

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
