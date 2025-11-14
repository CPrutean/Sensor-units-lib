#ifndef DEBUG
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#endif

#ifndef DEBUG
class gps_sensor {
public:
  gps_sensor(TinyGPSPlus &gps, HardwareSerial &gpsSerial, bool hasPPS = false,
             uint8_t ppsPin = 255);
  double pullLat();
  double pullLong();
  gps_sensor();
  bool isValidInstance();
  bool isFunctioning();

private:
  TinyGPSPlus *gps = nullptr;
  HardwareSerial *gpsSerial = nullptr;
};
#else

class gps_sensor {
public:
  gps_sensor();
  double pullLat();
  double pullLong();
  bool isValidInstance();
  bool isFunctioning();
  bool flipTestState();

private:
  bool isValid;
};

#endif
