#ifndef _DRV_BUZZER_H
#define _DRV_BUZZER_H

#include "pico/stdlib.h"
#include <stdint.h>

namespace drv
{

class buzzer
{
public:
    enum class Chime { BeepFast, BeepMedium, BeepSlow, Chirp };

public:
    buzzer(uint gpio);
    ~buzzer();

    // The pitch
    // 2700 is the loudest for this piezo buzzer
    void set_frequency_hz(uint32_t frequency);
    void set_volume_percentage(uint8_t volume_percent);
    // Sets volume to 0
    void stop();
    void play_blocking(Chime chime, uint32_t duration_ms, uint8_t volume_percent);

    // TODO: Make a non blocking implementation
    //bool is_playing();
    //void update();

private:
    uint _gpio;
    uint _channel;
    uint _slice_num;

    uint32_t _wrap_value_set;
}; // class buzzer

};     // namespace drv
#endif // _DRV_BUZZER_H