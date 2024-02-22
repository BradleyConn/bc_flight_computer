#include "parachute.h"

Parachute::Parachute(drv::servo& parachuteServo) : _parachuteServo(parachuteServo) {
    _parachuteServo.set_angle_milli_degrees(0);
}

Parachute::~Parachute() {
}

void Parachute::deploy() {
    // Positive 45 degrees deploys the parachute!
    // TODO: make this a constant somewhere
    _parachuteServo.set_angle_milli_degrees(45 * 1000);
}
