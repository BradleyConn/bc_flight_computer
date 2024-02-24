#ifndef TELEMETRY_CONTAINER_H
#define TELEMETRY_CONTAINER_H

#include "control_loop.h"
#include "drv_bmi088.h"
#include "drv_bmp280.h"
#include "drv_flash.h"
#include "orientation_calculator.h"
#include "state_detector.h"
#include "time_keeper.h"

namespace sys
{

struct PackagedTelemetryData {
    // start of the world timer
    char startOfTheWorld_id[3] = { 's', 'o', 'w' };
    uint64_t startOfTheWorld;

    char bmi088DatasetConverted_id[3] = { 'b', 'i', 'c' };
    bmi088DatasetConverted bmi088DatasetConvertedInst;
    char bmp280DatasetConverted_id[4] = { 'b', 'p', 'c' };
    bmp280DatasetConverted bmp280DatasetConvertedInst;
    // bmp should get the relative altitude too
    char relativeAltitude_cm_id[3] = { 'r', 'a', 'l' };
    int32_t relativeAltitude_cm;
    char stateFlags_id[3] = { 's', 'f', 'l' };
    StateDetector::StateFlags stateFlags;
    char orientation_id[3] = { 'o', 'r', 'n' };
    OrientationCalculator::QuaternionValues orientation;
    char controlLoop_id[3] = { 'c', 'l', 'p' };
    ControlLoop::ControlLoopData controlLoopData;
};

// if the size of the struct is bigger than the usable flash page size throw a compile time error
static_assert(sizeof(PackagedTelemetryData) <= drv::FlashDriver::usable_flash_page_size, "PackagedTelemetryData size is bigger than the flash page size");

// This class is a container for all the data that is collected from the sensors but it is
// packaged so that it can be logged to flash
// TODO: Initially this was intended to be an extension of logging but it didn't turn out that way
// There is tons of coupling to logging here that should be refactored and separated out
class TelemetryContainer
{
public:
    // Constructor zeros out the _rawLogBytes and fills in id's
    TelemetryContainer();

    // Destructor
    ~TelemetryContainer();

    // Getters and setters for BMI088 sensor data

    bmi088DatasetConverted getBMI088DatasetConverted() const;
    void setBMI088DatasetConverted(const bmi088DatasetConverted& bmi088DatasetConvertedInst);

    // Getters and setters for BMP280 sensor data

    bmp280DatasetConverted getBMP280DatasetConverted() const;
    void setBMP280DatasetConverted(const bmp280DatasetConverted& bmp280DatasetConvertedInst);

    // Getters and setters for relative altitude data

    int32_t getRelativeAltitude_cm() const;
    void setRelativeAltitude_cm(const int32_t& relativeAltitude_cm);

    // Getters and setters for state flags data

    StateDetector::StateFlags getStateFlags() const;
    void setStateFlags(const StateDetector::StateFlags& stateFlags);

    // Getters and setters for orientation data

    OrientationCalculator::QuaternionValues getOrientation() const;
    void setOrientation(const OrientationCalculator::QuaternionValues& orientation);

    // Getters and setters for control loop data

    ControlLoop::ControlLoopData getControlLoopData() const;
    void setControlLoopData(const ControlLoop::ControlLoopData& controlLoopData);

    // Getters and setters for time data
    //uint64_t getTimeData1() const;
    //void setTimeData1(const uint64_t& timeData);

    //start of the world timer
    uint64_t getStartOfTheWorld() const;
    void setStartOfTheWorld(const uint64_t& startOfTheWorld);

    // The functions that return the packaged data in various ways and formats
    PackagedTelemetryData* getPackagedTelemetryData() const;
    PackagedTelemetryData getPackagedTelemetryDataCopy() const;
    void setPackagedTelemetryData(const PackagedTelemetryData& packagedTelemetryDataInst);
    // This maybe should return const but provide extra flexibility for now
    uint8_t* getPackagedRawBytes();
    void getPackagedRawBytesCopy(uint8_t* packagedBytes) const;
    void setPackagedRawBytes(const uint8_t* packagedBytes);

    // Print functions
    void printRawLogBytes() const;
    void printRawLogBytes(const uint8_t* rawLogBytes) const;
    void printPackagedTelemetryData() const;
    void printPackagedTelemetryData(const PackagedTelemetryData& packagedTelemetryDataInst) const;

    static constexpr size_t rawLogBytesSize = drv::FlashDriver::usable_flash_page_size;

private:
    // The whole point of this is to be able to package telemtry for logging.
    // So first create the page array for logging which is a fixed size bigger than the telemetry data.
    // Then create an interface to that page array that is the telemetry struct
    // This way everything is fast and in place
    uint8_t _rawLogBytes[rawLogBytesSize];
    PackagedTelemetryData* _packagedTelemetryDataInst = reinterpret_cast<PackagedTelemetryData*>(_rawLogBytes);
};
} // namespace sys

#endif // TELEMETRY_CONTAINER_H
