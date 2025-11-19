#include "gps_sensor.h"
#ifndef DEBUG
gps_sensor::gps_sensor(TinyGPSPlus &gps, HardwareSerial &gpsSerial, bool hasPPS, uint8_t ppsPin)
:m_gps{gps}, m_gpsSerial{gpsSerial}
{

}

double gps_sensor::pullLat() {
  if (!this->isFunctioning()) {
    return static_cast<double>(NULL);
  }
  return static_cast<float>(this->m_gps.location.lat());
}

double gps_sensor::pullLong() {
  if (this->isFunctioning()) {
    return static_cast<double>(NULL);
  }
  return this->m_gps.location.lng();
}


bool gps_sensor::isFunctioning() {
  return this->pullLat() != 0.0 && this->pullLong() != 0.0;
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
