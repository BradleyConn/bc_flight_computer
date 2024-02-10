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

    servo(uint gpio, servo_type type, int64_t offset_milli_degree, int64_t starting_angle_millii_degrees = 0);
    ~servo();

    // Set the cent-degree angle! The degree * 100. So 4.32 is 432
    // This is to avoid floats.
    // This is between -90 and +90 so maximum -9000 to +9000
    void set_angle_centi_degrees(int64_t centi_degree);
    // Set the angle in milli degrees
    void set_angle_milli_degrees(int64_t milli_degree);
    void turn_off();

private:
    uint _gpio;
    uint _channel;
    uint _slice_num;
    servo_type _type;
    int64_t _offset_milli_degree = 0;
}; // class servo

}; // namespace drv
#endif
