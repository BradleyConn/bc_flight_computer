#include <stdio.h>
#include "../inc/drv_bmi088.h"

// Implementation stripped almost exactly from the the pico examples
namespace drv
{
    bmi088::bmi088(uint sclk, uint miso, uint mosi, uint accel_cs, uint gyro_cs, spi_module_num spi_module)
        : _sclk(sclk), _miso(miso), _mosi(mosi), _accel_cs(accel_cs), _gyro_cs(gyro_cs)
    {

        if (spi_module == spi_module_0)
        {
            _spi_inst = spi0;
        }
        else if (spi_module == spi_module_1)
        {
            _spi_inst = spi1;
        }
        // 0.5MHz (bmi can handle 10mhz)
        spi_init(_spi_inst, 500 * 1000);
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
    }

    // TODO: Unset pins
    bmi088::~bmi088()
    {
    }

    uint8_t bmi088::readAccelID()
    {
        uint8_t id;
        accel_read_registers(0x00, &id, 1);
        printf("Chip ID is 0x%x\n", id);
        return id;
    }
    uint8_t bmi088::readGyroID()
    {
        uint8_t id;
        gyro_read_registers(0x00, &id, 1);
        printf("Chip ID is 0x%x\n", id);
        return id;
    }

    void bmi088::print_reg(uint8_t reg)
    {
        uint8_t reg_val = 0xa5;
        accel_read_registers(reg, &reg_val, 1);
        printf("Reg 0x%02X, Val = 0x%02X\n", reg, reg_val);
    }

    void bmi088::print_reg_expected(uint8_t reg, uint8_t expected)
    {
        uint8_t reg_val = 0xa5;
        accel_read_registers(reg, &reg_val, 1);
        printf("Reg 0x%02X, expected = 0x%02X, Val = 0x%02X\n", reg, expected, reg_val);
    }

    void bmi088::init()
    {
        sleep_ms(1);
        // Do a dummy read to set the accel to SPI mode.
        puts("Dummy read expect 0x00");
        readAccelID();
        accel_write_register(0x7E, 0xB6);
        sleep_ms(50);
        // Do a dummy read to set the accel to SPI mode.
        puts("Dummy read expect 0x00");
        readAccelID();

        // TODO: better error handling.
        while (readAccelID() != 0x1E)
        {
            puts("error!");
        }
        if (readGyroID() != 0x0F)
        {
            puts("error!");
        }

        print_reg_expected(0x7D, 0x00);
        print_reg_expected(0x7C, 0x03);
        print_reg_expected(0x6D, 0x00);
        print_reg_expected(0x58, 0x00);
        print_reg_expected(0x54, 0x00);
        print_reg_expected(0x53, 0x00);
        print_reg_expected(0x49, 0x10);
        print_reg_expected(0x48, 0x02);
        print_reg_expected(0x47, 0x02);
        print_reg_expected(0x46, 0x00);
        print_reg_expected(0x45, 0x80);
        print_reg_expected(0x41, 0x01);
        print_reg_expected(0x40, 0xA8);
        print_reg_expected(0x03, 0x00);
        print_reg_expected(0x02, 0x00);
        print_reg_expected(0x00, 0x1E);

        // Set 4 to reg 0x7D to turn on accelerometer
        accel_write_register(0x7D, 0x04);
        sleep_ms(50);

        // Now configure it
        // Set the sample rate to 145hz - 1600 ODR, 4x oversampling
        accel_write_register(0x40, 0x8C);
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
        // Set the interrupt 3 pin to active
        gyro_write_register(0x17, 0x01);
    }

    void bmi088::getData()
    {
        uint8_t data_u8[9];
        int16_t data_i16[9];
        for (int i = 0; i < sizeof(data_u8); i++)
        {
            data_i16[i] = 0;
            data_u8[i] = 0;
        }

        // accel_read_registers(0x12, data_u8, sizeof(data_u8));
        // accel_read_registers(0x12, &(data_u8[0]), 1);
        // accel_read_registers(0x13, &(data_u8[1]), 1);
        // accel_read_registers(0x14, &(data_u8[2]), 1);
        // accel_read_registers(0x15, &(data_u8[3]), 1);
        // accel_read_registers(0x16, &(data_u8[4]), 1);
        // accel_read_registers(0x17, &(data_u8[5]), 1);
        accel_read_registers(0x12, data_u8, 9);

        // uint8_t time = 0;

        // accel_read_registers(0x1A, &time, 1);

        for (int i = 0; i < sizeof(data_u8); i++)
        {
            data_i16[i] = data_u8[i];
        }
        printf("X = %d\n", (int16_t)((data_i16[1] << 8) + data_i16[0]));
        printf("Y = %d\n", (int16_t)((data_i16[3] << 8) + data_i16[2]));
        printf("Z = %d\n", (int16_t)((data_i16[5] << 8) + data_i16[4]));
        // printf("X MSB, LSB = %d, %d, time = %d\n", data_u8[1], data_u8[0], time);
        printf("X HSB, MSB, LSB = %03d, %03d, %03d\n", data_u8[8], data_u8[7], data_u8[6]);

        return;
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
        if (cs == accel)
        {
            accel_cs_select();
        }
        else if (cs == gyro)
        {
            gyro_cs_select();
        }
        spi_write_blocking(_spi_inst, buf, 2);
        accel_cs_deselect();
        gyro_cs_deselect();
        sleep_ms(10);
    }

#define READ_BIT 0x80
    void bmi088::accel_read_registers(uint8_t reg, uint8_t *buf, uint16_t len)
    {
        // The accelerometer needs a dummy read first unlike the gyro :facepalm:
        reg |= READ_BIT;
        accel_cs_select();
        spi_write_blocking(_spi_inst, &reg, 1);
        sleep_ms(10);
        spi_read_blocking(_spi_inst, 0, buf, 1);
        spi_read_blocking(_spi_inst, 0, buf, len);
        accel_cs_deselect();
        sleep_ms(10);
    }
    void bmi088::gyro_read_registers(uint8_t reg, uint8_t *buf, uint16_t len)
    {
        read_registers(reg, buf, len, gyro);
    }
    void bmi088::read_registers(uint8_t reg, uint8_t *buf, uint16_t len, cs_type cs)
    {
        // For this particular device, we send the device the register we want to read
        // first, then subsequently read from the device. The register is auto incrementing
        // so we don't need to keep sending the register we want, just the first.
        reg |= READ_BIT;
        if (cs == accel)
        {
            accel_cs_select();
        }
        else if (cs == gyro)
        {
            gyro_cs_select();
        }
        spi_write_blocking(_spi_inst, &reg, 1);
        sleep_ms(10);
        spi_read_blocking(_spi_inst, 0, buf, len);
        accel_cs_deselect();
        gyro_cs_deselect();
        sleep_ms(10);
    }
} // namespace drv
