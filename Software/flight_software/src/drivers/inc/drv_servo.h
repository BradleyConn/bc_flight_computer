#ifndef _DRV_SERVO_H
#define _DRV_SERVO_H

#include "pico/stdlib.h"

namespace drv
{

class servo
{
public:
    enum servo_type {
        Analog = 0x00,
        Digital = 0x01,
    };

    servo(uint gpio, servo_type type, int32_t starting_angle_centi_degrees);
    ~servo();

    // Set the cent-degree angle! The degree * 100. So 4.32 is 432
    // This is to avoid floats.
    void set_angle_centi_degrees(int32_t centi_degree);

private:
    uint _gpio;
    uint _channel;
    uint _slice_num;
    servo_type _type;
}; // class servo

}; // namespace drv
#endif
