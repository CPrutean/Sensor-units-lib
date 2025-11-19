#include "temperature_sensor.h"
#include <cfloat>
#include <cmath>

#ifndef DEBUG
float temperature_sensor::pullTemp() {
  return m_dht.readTemperature(pref != 'C');
}

float temperature_sensor::pullHumid() {
    return m_dht.readHumidity();
}

char temperature_sensor::pullPref() { return this->pref; }

void temperature_sensor::pushPref(char pref) {
  if (pref == 'C' || pref == 'F') {
    this->pref = pref;
  } else {
#ifdef DEBUG
    Serial.println("INVALID TYPE PASSED");
#endif
  }
}

bool temperature_sensor::isFunctioning() {
  return this->pullTemp() != NAN && this->pullHumid() != NAN;
}
#else
float temperature_sensor::pullTemp() {
  return this->isValid ? 23.1 : 0.0;
}

float temperature_sensor::pullHumid() {
  return this->isValid ? 22.1 : 0.0;
}

char temperature_sensor::pullPref() {
  return this->pref;
}

void temperature_sensor::pushPref(char pref) {
  if (pref == 'C' || pref == 'F') {
    this->pref = pref;
  } else {
    return;
  }
}

temperature_sensor::temperature_sensor() {
  this->pref = 'C';
  this->isValid = true;
}

bool temperature_sensor::flipTestState() {
  this->isValid = !this->isValid;
  return this->isValid;
}

#endif
