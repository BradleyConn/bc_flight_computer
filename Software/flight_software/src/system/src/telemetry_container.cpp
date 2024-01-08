#include "telemetry_container.h"
#include <cstring>
#include <stdio.h>

namespace sys
{
TelemetryContainer::TelemetryContainer()
{
    // Constructor zeros out the _rawLogBytes
    memset(_rawLogBytes, 0, sizeof(_rawLogBytes));
    // now put the id's back in because they were cleared by the memset
    // Create a new instance of the PackagedTelemetryData struct and copy it over
    auto x = PackagedTelemetryData();
    memcpy(_packagedTelemetryDataInst->timeData1_id, x.timeData1_id, sizeof(x.timeData1_id));
    memcpy(_packagedTelemetryDataInst->timeData2_id, x.timeData2_id, sizeof(x.timeData2_id));
    memcpy(_packagedTelemetryDataInst->timeData3_id, x.timeData3_id, sizeof(x.timeData3_id));
    memcpy(_packagedTelemetryDataInst->bmi088DatasetRaw_id, x.bmi088DatasetRaw_id, sizeof(x.bmi088DatasetRaw_id));
    memcpy(_packagedTelemetryDataInst->bmi088DatasetConverted_id, x.bmi088DatasetConverted_id, sizeof(x.bmi088DatasetConverted_id));
    memcpy(_packagedTelemetryDataInst->bmp280DatasetRaw_id, x.bmp280DatasetRaw_id, sizeof(x.bmp280DatasetRaw_id));
    memcpy(_packagedTelemetryDataInst->bmp280DatasetConverted_id, x.bmp280DatasetConverted_id, sizeof(x.bmp280DatasetConverted_id));
}

TelemetryContainer::~TelemetryContainer()
{
    // Destructor
}

bmi088DatasetRaw TelemetryContainer::getBMI088DatasetRaw() const
{
    return _packagedTelemetryDataInst->bmi088DatasetRawInst;
}

void TelemetryContainer::setBMI088DatasetRaw(const bmi088DatasetRaw& bmi088DatasetRawInst)
{
    _packagedTelemetryDataInst->bmi088DatasetRawInst = bmi088DatasetRawInst;
}

bmi088DatasetConverted TelemetryContainer::getBMI088DatasetConverted() const
{
    return _packagedTelemetryDataInst->bmi088DatasetConvertedInst;
}

void TelemetryContainer::setBMI088DatasetConverted(const bmi088DatasetConverted& bmi088DatasetConvertedInst)
{
    _packagedTelemetryDataInst->bmi088DatasetConvertedInst = bmi088DatasetConvertedInst;
}

bmp280DatasetRaw TelemetryContainer::getBMP280DatasetRaw() const
{
    return _packagedTelemetryDataInst->bmp280DatasetRawInst;
}

void TelemetryContainer::setBMP280DatasetRaw(const bmp280DatasetRaw& bmp280DatasetRawInst)
{
    _packagedTelemetryDataInst->bmp280DatasetRawInst = bmp280DatasetRawInst;
}

bmp280DatasetConverted TelemetryContainer::getBMP280DatasetConverted() const
{
    return _packagedTelemetryDataInst->bmp280DatasetConvertedInst;
}

void TelemetryContainer::setBMP280DatasetConverted(const bmp280DatasetConverted& bmp280DatasetConvertedInst)
{
    _packagedTelemetryDataInst->bmp280DatasetConvertedInst = bmp280DatasetConvertedInst;
}

uint64_t TelemetryContainer::getTimeData1() const
{
    return _packagedTelemetryDataInst->timeData1;
}

void TelemetryContainer::setTimeData1(const uint64_t& timeData)
{
    _packagedTelemetryDataInst->timeData1 = timeData;
}

uint64_t TelemetryContainer::getTimeData2() const
{
    return _packagedTelemetryDataInst->timeData2;
}

void TelemetryContainer::setTimeData2(const uint64_t& timeData)
{
    _packagedTelemetryDataInst->timeData2 = timeData;
}

uint64_t TelemetryContainer::getTimeData3() const
{
    return _packagedTelemetryDataInst->timeData3;
}

void TelemetryContainer::setTimeData3(const uint64_t& timeData)
{
    _packagedTelemetryDataInst->timeData3 = timeData;
}

PackagedTelemetryData* TelemetryContainer::getPackagedTelemetryData() const
{
    return _packagedTelemetryDataInst;
}

PackagedTelemetryData TelemetryContainer::getPackagedTelemetryDataCopy() const
{
    return *_packagedTelemetryDataInst;
}

void TelemetryContainer::setPackagedTelemetryData(const PackagedTelemetryData& packagedTelemetryDataInst)
{
    *_packagedTelemetryDataInst = packagedTelemetryDataInst;
}

uint8_t* TelemetryContainer::getPackagedRawBytes()
{
    return _rawLogBytes;
}

void TelemetryContainer::getPackagedRawBytesCopy(uint8_t* packagedBytes) const
{
    memcpy(packagedBytes, _rawLogBytes, sizeof(_rawLogBytes));
}

void TelemetryContainer::setPackagedRawBytes(const uint8_t* packagedBytes)
{
    memcpy(_rawLogBytes, packagedBytes, sizeof(_rawLogBytes));
}
// Print functions
void TelemetryContainer::printRawLogBytes() const
{
    printRawLogBytes(_rawLogBytes);
}

void TelemetryContainer::printRawLogBytes(const uint8_t* rawLogBytes) const
{
    printf("Raw Log Bytes: ");
    for (size_t i = 0; i < sizeof(_rawLogBytes); i++) {
        printf("%02X ", rawLogBytes[i]);
    }
    printf("\n");
}

void TelemetryContainer::printPackagedTelemetryData() const
{
    printPackagedTelemetryData(*_packagedTelemetryDataInst);
}

void TelemetryContainer::printPackagedTelemetryData(const PackagedTelemetryData& packagedTelemetryDataInst) const
{
    printf("Packaged Telemetry Data:\n");
    printf("Time Data 1: %llu\n", packagedTelemetryDataInst.timeData1);
    printf("Time Data 2: %llu\n", packagedTelemetryDataInst.timeData2);
    printf("Time Data 3: %llu\n", packagedTelemetryDataInst.timeData3);
    printf("BMI088 Raw Data:\n");
    drv::bmi088::print_data_raw(packagedTelemetryDataInst.bmi088DatasetRawInst);
    printf("BMI088 Converted Data:\n");
    drv::bmi088::print_data_converted(packagedTelemetryDataInst.bmi088DatasetConvertedInst);
    printf("BMP280 Raw Data:\n");
    drv::bmp280::print_data_raw(packagedTelemetryDataInst.bmp280DatasetRawInst);
    printf("BMP280 Converted Data:\n");
    drv::bmp280::print_data_converted(packagedTelemetryDataInst.bmp280DatasetConvertedInst);
}

} // namespace sys
