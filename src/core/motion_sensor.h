#include <PIR.h>

#ifndef DEBUG
class motion_sensor {
public:
  motion_sensor(PIR &sensor);
  float pullMotion();
  motion_sensor();
  bool isValidInstance();
  bool isFunctioning();

private:
  PIR *motion = nullptr;

};
#else
class motion_sensor {
  public:
    motion_sensor();
    float pullMotion();
    bool isValidInstance();
    bool isFunctioning();
    bool flipTestState();

  private:
    bool isValid;
  };
#endif
