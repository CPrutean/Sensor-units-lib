#include "sensor_classes.h"

motion_sensor::motion_sensor(PIR &sensor) { this->motion = &sensor; }

float motion_sensor::pullMotion() {
  if (this->motion != nullptr) {
    return this->motion->read();
  } else {
    return 255;
  }
}
