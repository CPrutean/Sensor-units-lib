#include <DHT.h>
#ifndef DEBUG
class temperature_sensor {
public:
  float pullTemp();
  float pullHumid();
  char pullPref();
  void pushPref(char pref);
  explicit temperature_sensor(DHT &dht): m_dht{dht} {}
  bool isFunctioning();
private:
  char pref = 'C';
  DHT &m_dht;
};
#else
class temperature_sensor {
  public:
    float pullTemp();
    float pullHumid();
    char pullPref();
    void pushPref(char pref);
    temperature_sensor();
    bool isValidInstance();
    bool isFunctioning();
    bool flipTestState();

  private:
    char pref = 'C';
    bool isValid = true;
};
#endif