#include "gps_sensor.h"
#ifndef DEBUG
gps_sensor::gps_sensor(TinyGPSPlus &gps, HardwareSerial &gpsSerial, bool hasPPS,
                       uint8_t ppsPin) {
  this->gps = &gps;
  this->gpsSerial = &gpsSerial;
}
gps_sensor::gps_sensor() {
  this->gps = nullptr;
  this->gpsSerial = nullptr;
}
double gps_sensor::pullLat() {
  if (this->isValidInstance()) {
    return static_cast<double>(NULL);
  }
  return static_cast<float>(this->gps->location.lat());
}

double gps_sensor::pullLong() {
  if (this->isValidInstance()) {
    return static_cast<double>(NULL);
  }
  return this->gps->location.lng();
}

bool gps_sensor::isValidInstance() {
  return this->gps != nullptr && this->gpsSerial != nullptr;
}

bool gps_sensor::isFunctioning() {
  return this->pullLat() != 0.0 && this->pullLong() != pullLong();
}
#else
gps_sensor::gps_sensor() {
  this->isValid = true;
}

double gps_sensor::pullLat() {
  return this->isValid ? 20.4 : 0.0;
}

double gps_sensor::pullLong() {
  return this->isValid ? 23.1 : 0.0;
}

bool gps_sensor::isValidInstance() {
  return this->isValid;
}

bool gps_sensor::isFunctioning() {
  return this->isValid;
}

bool gps_sensor::changeTestState() {
  this->isValid = !this->isValid;
  return this->isValid;
}


#endif
