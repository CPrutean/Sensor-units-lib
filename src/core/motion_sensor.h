#include <PIR.h>

#ifndef DEBUG
class motion_sensor {
public:
  motion_sensor(PIR &sensor);
  float pullMotion();
  bool isFunctioning();

private:
  PIR &m_motion;
};

#else
class motion_sensor {
  public:
    motion_sensor();
    float pullMotion();
    bool isFunctioning();
    bool flipTestState();

  private:
    bool isValid;
  };
#endif
