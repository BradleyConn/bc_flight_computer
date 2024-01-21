#include "time_keeper.h"
#include "hardware/timer.h"
#include <stdio.h>


TimeKeeper::TimeKeeper()
{
    valid = false;
    markTime = 0;
}

bool TimeKeeper::isValid() const
{
    return valid;
}

void TimeKeeper::mark()
{
    markTime = time_us_64();
    valid = true;
}

uint64_t TimeKeeper::deltaTime_us() const
{
    return time_us_64() - markTime;
}

void TimeKeeper::printTimeuS() const
{
    printf("TimeKeeper: %llu us\n", deltaTime_us());
}

void TimeKeeper::printTimeMS() const
{
    printf("TimeKeeper: %llu ms\n", deltaTime_us() / 1000);
}

void TimeKeeper::printTimeS() const
{
    printf("TimeKeeper: %llu s\n", deltaTime_us() / 1000000);
}

uint64_t TimeKeeper::getAbsoluteTime_us()
{
    return time_us_64();
}
