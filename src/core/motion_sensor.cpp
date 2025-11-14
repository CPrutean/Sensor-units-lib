#include "motion_sensor.h"
#ifndef DEBUG
motion_sensor::motion_sensor(PIR &sensor) { this->motion = &sensor; }
motion_sensor::motion_sensor() { this->motion = nullptr; }
float motion_sensor::pullMotion() {
  if (this->motion != nullptr) {
    return this->motion->read();
  } else {
    return static_cast<float>(NULL);
  }
}

bool motion_sensor::isValidInstance() { return this->motion != nullptr; }

bool motion_sensor::isFunctioning() { return this->pullMotion(); }

#else
float motion_sensor::pullMotion() {
  return 1;
}

motion_sensor::motion_sensor() {
  this->isValid = true;
}

bool motion_sensor::isValidInstance() {
  return this->isValid;
}

bool motion_sensor::isFunctioning() {
  return this->isValid;
}

bool motion_sensor::flipTestState() {
  this->isValid = !this->isValid;
  return this->isValid;
}

#endif
