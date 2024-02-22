#ifndef PARACHUTE_H
#define PARACHUTE_H

//include servo header
#include "../../drivers/inc/drv_servo.h"

class Parachute {
public:
    Parachute(drv::servo& parachuteServo);
    ~Parachute();

    void deploy();

private:
    //servo reference to deploy the parachute
    drv::servo& _parachuteServo;
};

#endif // PARACHUTE_H
