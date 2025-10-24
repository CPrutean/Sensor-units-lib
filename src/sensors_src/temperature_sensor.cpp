#include "sensor_classes.h"
#include <cfloat>
#include <cmath>
temperature_sensor::temperature_sensor(TEMP_SENSOR_TYPE temp_sensor, DHT *dht) {
  this->type = temp_sensor;
  this->dht = dht;
}
temperature_sensor::temperature_sensor() {
  this->type = static_cast<TEMP_SENSOR_TYPE>(0);
  this->dht = nullptr;
}
float temperature_sensor::pullTemp() {
  if (type == DHT_SENSOR && dht != nullptr) {
    return dht->readTemperature(pref != 'C');
  } else {
    return 0.0f;
  }
}

float temperature_sensor::pullHumid() {
  if (type == DHT_SENSOR && dht != nullptr) {
    return dht->readHumidity();
  } else {
    return 0.0f;
  }
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

bool temperature_sensor::isValidInstance() { return this->dht != nullptr; }

bool temperature_sensor::isFunctioning() {
  return this->pullTemp() != NAN && this->pullHumid() != NAN;
}
