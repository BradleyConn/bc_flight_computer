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
    //memcpy(_packagedTelemetryDataInst->timeData1_id, x.timeData1_id, sizeof(x.timeData1_id));
    memcpy(_packagedTelemetryDataInst->bmi088DatasetConverted_id, x.bmi088DatasetConverted_id, sizeof(x.bmi088DatasetConverted_id));
    memcpy(_packagedTelemetryDataInst->bmp280DatasetConverted_id, x.bmp280DatasetConverted_id, sizeof(x.bmp280DatasetConverted_id));
    memcpy(_packagedTelemetryDataInst->relativeAltitude_cm_id, x.relativeAltitude_cm_id, sizeof(x.relativeAltitude_cm_id));
    memcpy(_packagedTelemetryDataInst->stateFlags_id, x.stateFlags_id, sizeof(x.stateFlags_id));
    memcpy(_packagedTelemetryDataInst->orientation_id, x.orientation_id, sizeof(x.orientation_id));
    memcpy(_packagedTelemetryDataInst->controlLoop_id, x.controlLoop_id, sizeof(x.controlLoop_id));
}

TelemetryContainer::~TelemetryContainer()
{
    // Destructor
}

bmi088DatasetConverted TelemetryContainer::getBMI088DatasetConverted() const
{
    return _packagedTelemetryDataInst->bmi088DatasetConvertedInst;
}

void TelemetryContainer::setBMI088DatasetConverted(const bmi088DatasetConverted& bmi088DatasetConvertedInst)
{
    _packagedTelemetryDataInst->bmi088DatasetConvertedInst = bmi088DatasetConvertedInst;
}

bmp280DatasetConverted TelemetryContainer::getBMP280DatasetConverted() const
{
    return _packagedTelemetryDataInst->bmp280DatasetConvertedInst;
}

void TelemetryContainer::setBMP280DatasetConverted(const bmp280DatasetConverted& bmp280DatasetConvertedInst)
{
    _packagedTelemetryDataInst->bmp280DatasetConvertedInst = bmp280DatasetConvertedInst;
}

int32_t TelemetryContainer::getRelativeAltitude_cm() const
{
    return _packagedTelemetryDataInst->relativeAltitude_cm;
}

void TelemetryContainer::setRelativeAltitude_cm(const int32_t& relativeAltitude_cm)
{
    _packagedTelemetryDataInst->relativeAltitude_cm = relativeAltitude_cm;
}

StateDetector::StateFlags TelemetryContainer::getStateFlags() const
{
    return _packagedTelemetryDataInst->stateFlags;
}

void TelemetryContainer::setStateFlags(const StateDetector::StateFlags& stateFlags)
{
    _packagedTelemetryDataInst->stateFlags = stateFlags;
}

OrientationCalculator::QuaternionValues TelemetryContainer::getOrientation() const
{
    return _packagedTelemetryDataInst->orientation;
}

void TelemetryContainer::setOrientation(const OrientationCalculator::QuaternionValues& orientation)
{
    _packagedTelemetryDataInst->orientation = orientation;
}

ControlLoop::ControlLoopData TelemetryContainer::getControlLoopData() const
{
    return _packagedTelemetryDataInst->controlLoopData;
}

void TelemetryContainer::setControlLoopData(const ControlLoop::ControlLoopData& controlLoopData)
{
    _packagedTelemetryDataInst->controlLoopData = controlLoopData;
}

uint64_t TelemetryContainer::getStartOfTheWorld() const
{
    return _packagedTelemetryDataInst->startOfTheWorld;
}

void TelemetryContainer::setStartOfTheWorld(const uint64_t& startOfTheWorld)
{
    _packagedTelemetryDataInst->startOfTheWorld = startOfTheWorld;
}



#if 0
uint64_t TelemetryContainer::getTimeData1() const
{
    return _packagedTelemetryDataInst->timeData1;
}

void TelemetryContainer::setTimeData1(const uint64_t& timeData)
{
    _packagedTelemetryDataInst->timeData1 = timeData;
}

PackagedTelemetryData* TelemetryContainer::getPackagedTelemetryData() const
{
    return _packagedTelemetryDataInst;
}
#endif

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
    //printf("Time Data 1: %llu\n", packagedTelemetryDataInst.timeData1);
    printf("BMI088 Converted Data:\n");
    drv::bmi088::print_data_converted(packagedTelemetryDataInst.bmi088DatasetConvertedInst);
    printf("BMP280 Converted Data:\n");
    drv::bmp280::print_data_converted(packagedTelemetryDataInst.bmp280DatasetConvertedInst);
    printf("Relative Altitude: %ld cm\n", packagedTelemetryDataInst.relativeAltitude_cm);
    printf("State Flags:\n");
    printf("Liftoff: %d\n", packagedTelemetryDataInst.stateFlags.liftoff_detected);
    printf("Burnout: %d\n", packagedTelemetryDataInst.stateFlags.burnout_detected);
    printf("Apogee: %d\n", packagedTelemetryDataInst.stateFlags.apogee_detected);
    printf("Landing: %d\n", packagedTelemetryDataInst.stateFlags.landing_detected);
    printf("Abort: %d\n", packagedTelemetryDataInst.stateFlags.abort);
    printf("Orientation:\n");
    printf("a: %f, b: %f, c: %f, d: %f\n", packagedTelemetryDataInst.orientation.a, packagedTelemetryDataInst.orientation.b,
           packagedTelemetryDataInst.orientation.c, packagedTelemetryDataInst.orientation.d);
    printf("Control Loop Data:\n");
    printf("YawErrorDegrees: %f\n", packagedTelemetryDataInst.controlLoopData.yaw_error_degrees);
    printf("PitchErrorDegrees: %f\n", packagedTelemetryDataInst.controlLoopData.pitch_error_degrees);
    printf("yaw_PID_angle: %f\n", packagedTelemetryDataInst.controlLoopData.yaw_PID_angle);
    printf("pitch_PID_angle: %f\n", packagedTelemetryDataInst.controlLoopData.pitch_PID_angle);
    printf("yaw_tvc_angle: %f\n", packagedTelemetryDataInst.controlLoopData.yaw_tvc_angle);
    printf("pitch_tvc_angle: %f\n", packagedTelemetryDataInst.controlLoopData.pitch_tvc_angle);
    printf("yaw_servo_setpoint: %f\n", packagedTelemetryDataInst.controlLoopData.yaw_servo_setpoint);
    printf("pitch_servo_setpoint: %f\n", packagedTelemetryDataInst.controlLoopData.pitch_servo_setpoint);
    printf("Time since last update: %lu us\n", packagedTelemetryDataInst.controlLoopData.time_since_update_us);
    printf("Time under thrust: %lu ms\n", packagedTelemetryDataInst.controlLoopData.time_under_thrust_ms);

    printf("Start of the world: %llu\n", packagedTelemetryDataInst.startOfTheWorld);

    printf("\n");



}

} // namespace sys
