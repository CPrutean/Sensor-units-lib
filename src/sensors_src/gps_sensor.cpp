#include "sensor_classes.h"

gps_sensor::gps_sensor(TinyGPSPlus &gps, HardwareSerial &gpsSerial, bool hasPPS,
                       uint8_t ppsPin) {
  this->gps = &gps;
  this->gpsSerial = &gpsSerial;
}

float gps_sensor::pullLat() {
  return static_cast<float>(this->gps->location.lat());
}

float gps_sensor::pullLong() {
  return static_cast<float>(this->gps->location.lng());
}
