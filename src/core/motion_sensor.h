#include "motion_sensor.h"
#include <PIR.h>

class motion_sensor {
public:
  motion_sensor(PIR &sensor);
  float pullMotion();
  motion_sensor();
  bool isValidInstance();
  bool isFunctioning();

private:
#ifndef DEBUG
  PIR *motion = nullptr;
#else
  class motion_sensor {
  public:
    motion_sensor();
    float pullMotion();
    bool isValidInstance();
    bool isFunctioning();
    bool flipState();

  private:
    bool functioningState;
  }
#endif
};
