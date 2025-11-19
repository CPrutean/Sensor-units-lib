#ifndef DEBUG
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#endif

#ifndef DEBUG
class gps_sensor {
public:
  gps_sensor(TinyGPSPlus &gps, HardwareSerial &gpsSerial, bool hasPPS = false, uint8_t ppsPin = 255); //Gps sensors can't be default constructed
  double pullLat();
  double pullLong();
  bool isFunctioning();

private:
  TinyGPSPlus &m_gps;
  HardwareSerial &m_gpsSerial;
};
#else

class gps_sensor {
public:
  gps_sensor();
  double pullLat();
  double pullLong();
  bool isFunctioning();
  bool flipTestState();

private:
  bool isValid;
};

#endif
