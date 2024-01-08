#include <stdint.h>
class IThrustCurve
{
public:
    virtual uint32_t get_thrust_mN(uint32_t time_ms) = 0;
};