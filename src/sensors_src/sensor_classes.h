#include <DHT.h>
#include <HardwareSerial.h>
#include <PIR.h>
#include <TinyGPS++.h>

enum TEMP_SENSOR_TYPE { DHT_SENSOR, NONE };

class temperature_sensor {
public:
  float pullTemp();
  float pullHumid();
  char pullPref();
  void pushPref(char pref);
  temperature_sensor(TEMP_SENSOR_TYPE sensor_type, DHT *dht = nullptr);

private:
  char pref = 'C';
  DHT *dht = nullptr;
  TEMP_SENSOR_TYPE type;
};

class gps_sensor {
public:
  gps_sensor(TinyGPSPlus &gps, HardwareSerial &gpsSerial, bool hasPPS = false,
             uint8_t ppsPin = 255);
  float pullLat();
  float pullLong();

private:
  TinyGPSPlus *gps = nullptr;
  HardwareSerial *gpsSerial = nullptr;
};

class motion_sensor {
public:
  motion_sensor(PIR &sensor);
  float pullMotion();

private:
  PIR *motion = nullptr;
};
