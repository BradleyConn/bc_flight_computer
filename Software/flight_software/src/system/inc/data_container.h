#ifndef DATA_CONTAINER_H
#define DATA_CONTAINER_H

#include "drv_bmi088.h"
#include "drv_bmp280.h"
#include "drv_flash.h"
#include "time_keeper.h"

namespace sys
{

struct PackagedTelemetryData {
    char timeData1_id[3] = { 't', 'm', '1' };
    uint64_t timeData1;
    char timeData2_id[3] = { 't', 'm', '2' };
    uint64_t timeData2;
    char timeData3_id[3] = { 't', 'm', '3' };
    uint64_t timeData3;
    char bmi088DatasetRaw_id[3] = { 'b', 'i', 'r' };
    bmi088DatasetRaw bmi088DatasetRawInst;
    char bmi088DatasetConverted_id[3] = { 'b', 'i', 'c' };
    bmi088DatasetConverted bmi088DatasetConvertedInst;
    char bmp280DatasetRaw_id[3] = { 'b', 'p', 'r' };
    bmp280DatasetRaw bmp280DatasetRawInst;
    char bmp280DatasetConverted_id[4] = { 'b', 'p', 'c' };
    bmp280DatasetConverted bmp280DatasetConvertedInst;
};

// This class is a container for all the data that is collected from the sensors but it is packaged so that it can be logged to flash
class DataContainer
{
public:
    // Constructor zeros out the _rawLogBytes and fills in id's
    DataContainer();

    // Destructor
    ~DataContainer();

    // Getters and setters for BMI088 sensor data
    bmi088DatasetRaw getBMI088DatasetRaw() const;
    void setBMI088DatasetRaw(const bmi088DatasetRaw& bmi088DatasetRawInst);
    bmi088DatasetConverted getBMI088DatasetConverted() const;
    void setBMI088DatasetConverted(const bmi088DatasetConverted& bmi088DatasetConvertedInst);

    // Getters and setters for BMP280 sensor data
    bmp280DatasetRaw getBMP280DatasetRaw() const;
    void setBMP280DatasetRaw(const bmp280DatasetRaw& bmp280DatasetRawInst);
    bmp280DatasetConverted getBMP280DatasetConverted() const;
    void setBMP280DatasetConverted(const bmp280DatasetConverted& bmp280DatasetConvertedInst);

    // Getters and setters for time data
    uint64_t getTimeData1() const;
    void setTimeData1(const uint64_t& timeData);
    uint64_t getTimeData2() const;
    void setTimeData2(const uint64_t& timeData);
    uint64_t getTimeData3() const;
    void setTimeData3(const uint64_t& timeData);

    // The functions that return the packaged data in various ways and formats
    PackagedTelemetryData* getPackagedTelemetryData() const;
    PackagedTelemetryData getPackagedTelemetryDataCopy() const;
    void setPackagedTelemetryData(const PackagedTelemetryData& packagedTelemetryDataInst);
    // This maybe should return const but provide extra flexibility for now
    uint8_t* getPackagedRawBytes();
    void getPackagedRawBytesCopy(uint8_t* packagedBytes) const;

    // Print functions
    void printRawLogBytes() const;
    void printRawLogBytes(const uint8_t* rawLogBytes) const;
    void printPackagedTelemetryData() const;
    void printPackagedTelemetryData(const PackagedTelemetryData& packagedTelemetryDataInst) const;

private:
    // The whole point of this is to be able to package telemtry for logging.
    // So first create the page array for logging which is a fixed size bigger than the telemetry data.
    // Then create an interface to that page array that is the telemetry struct
    // This way everything is fast and in place
    uint8_t _rawLogBytes[drv::FlashDriver::usable_flash_page_size];
    PackagedTelemetryData* _packagedTelemetryDataInst = reinterpret_cast<PackagedTelemetryData*>(_rawLogBytes);
};
} // namespace sys

#endif // DATA_CONTAINER_H
