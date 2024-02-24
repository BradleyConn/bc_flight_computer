#include "../inc/logger.h"
#include <string.h>

Logger::Logger(drv::FlashDriver& flash_driver) : _flash_driver(flash_driver)
{
}

Logger::~Logger()
{
}

void Logger::update(sys::TelemetryContainer& telemetry)
{
    // check the bounds
    if (_telemetry_index >= _telemetry_buffer_size) {
        return;
    }
    // copy the telemetry container into the buffer
    memcpy(&_telemetry_containers_buff[_telemetry_index], telemetry.getPackagedRawBytes(), sys::TelemetryContainer::rawLogBytesSize);
    // increment the index
    _telemetry_index++;
    
    //printf ("Logger::update telemetry_index = %d\n", _telemetry_index);

}

void Logger::write_log_to_flash()
{
    _flash_driver.write_session_header();
    for (size_t i = 0; i < _telemetry_index; i++) {
        printf ("Logger::write_log_to_flash i = %d\n", i);
        auto telemetry = sys::TelemetryContainer();
        //get the ptr to the buffer
        const uint8_t* ptr = &_telemetry_containers_buff[i][0];
        //copy the buffer into the telemetry container
        telemetry.setPackagedRawBytes(ptr);
        telemetry.printRawLogBytes();
        telemetry.printPackagedTelemetryData();
        puts("Logger::write_log_to_flash - writing to flash");
        _flash_driver.write_next_usable_page_size(telemetry.getPackagedRawBytes());
        sleep_ms(100);
    }
}