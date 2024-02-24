#ifndef LOGGER_H
#define LOGGER_H

#include "../../drivers/inc/drv_flash.h"
#include "telemetry_container.h"
#include <stdint.h>
#include <string>

class Logger
{
public:
    Logger(drv::FlashDriver& flash_driver);
    ~Logger();

    //Logger is a singleton
    static Logger& get_instance();

    // This class holds a static buffer of telemetry containers. It increments them when update is called
    // and writes them all out to flash at the end of the flight
    void update(sys::TelemetryContainer& telemetry);

    // This function is called at the end of the flight to write the log to flash
    void write_log_to_flash();

private:
    // A static buffer of telemetry containers
    // 25 hz * 10 seconds = 250. At 256 bytes per telemetry container, that's 64k
    constexpr static size_t _telemetry_buffer_size = 10;//25 * 10;
    // The buffer
    uint8_t _telemetry_containers_buff[_telemetry_buffer_size][sys::TelemetryContainer::rawLogBytesSize];

    // The index of the next telemetry container to write to
    uint32_t _telemetry_index = 0;

    // The flash driver
    drv::FlashDriver& _flash_driver;
};

#endif // LOGGER_H
