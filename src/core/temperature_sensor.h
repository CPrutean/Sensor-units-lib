#include <DHT.h>
enum TEMP_SENSOR_TYPE { DHT_SENSOR, NONE };

class temperature_sensor {
public:
  float pullTemp();
  float pullHumid();
  char pullPref();
  void pushPref(char pref);
  temperature_sensor(TEMP_SENSOR_TYPE sensor_type, DHT *dht = nullptr);
  temperature_sensor();
  bool isValidInstance();
  bool isFunctioning();

private:
  char pref = 'C';
  TEMP_SENSOR_TYPE type;
#ifndef DEBUG
  DHT *dht = nullptr;
#else

#endif
};
