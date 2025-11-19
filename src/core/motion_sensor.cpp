#include "motion_sensor.h"
#ifndef DEBUG
motion_sensor::motion_sensor(PIR &sensor): m_motion{sensor} {

}

float motion_sensor::pullMotion() {
    return this->m_motion.read();
}

bool motion_sensor::isFunctioning() { return static_cast<bool>(this->pullMotion()); }

#else
float motion_sensor::pullMotion() {
  return isValid?1:0;
}

motion_sensor::motion_sensor() {
  this->isValid = true;
}

bool motion_sensor::isFunctioning() {
  return this->isValid;
}

bool motion_sensor::flipTestState() {
  this->isValid = !this->isValid;
  return this->isValid;
}

#endif
