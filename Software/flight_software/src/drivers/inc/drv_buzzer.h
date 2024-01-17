#ifndef _DRV_BUZZER_H
#define _DRV_BUZZER_H

#include "pico/stdlib.h"
#include <stdint.h>

namespace drv
{

class buzzer
{
public:
    buzzer(uint gpio);
    ~buzzer();

    // The pitch
    // 2700 is the loudest for this piezo buzzer
    void set_frequency_hz(uint32_t frequency);
    void set_volume_percentage(uint8_t volume_percent);

private:
    uint _gpio;
    uint _channel;
    uint _slice_num;

    uint32_t _wrap_value_set;
}; // class buzzer

}; // namespace drv
#endif
