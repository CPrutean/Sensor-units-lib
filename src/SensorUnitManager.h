#include "global_include.h"
#include <WiFi.h>
#include <esp_now.h>
#if __BYTE_ORDER__ == __LITTLE_ENDIAN__
#define ____BYTE_ORDER__ __ORDER_BIG__ORDER_BIG_ENDIAN__
#endif
class SensorUnitManager {
public:
  enum SensorUnitStatus { ONLINE = 0, ERROR, OFFLINE };
  SensorUnitManager(const SensorUnitManager &) = delete;
  virtual ~SensorUnitManager();
  SensorUnitManager();
  virtual void sendToSu(Packet packet, int suNum) = 0;
  virtual void handlePacket(const Packet &packet) = 0;

protected:
  uint8_t m_suMac[10][6]{};
  SensorUnitStatus m_status[10]{};
  unsigned long long msgID{};
};

class WifiSensUnitManager final : public SensorUnitManager {};
class UartSensUnitManager final : public SensorUnitManager {};
