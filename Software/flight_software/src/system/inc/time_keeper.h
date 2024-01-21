#ifndef TIME_KEEPER_H
#define TIME_KEEPER_H

#include <stdint.h>

class TimeKeeper
{
public:
    TimeKeeper();
    bool isValid() const;
    void mark();
    uint64_t deltaTime_us() const;
    void printTimeuS() const;
    void printTimeMS() const;
    void printTimeS() const;

    static uint64_t getAbsoluteTime_us();

private:
    uint64_t markTime;
    bool valid;
};

#endif // TIME_KEEPER_H
