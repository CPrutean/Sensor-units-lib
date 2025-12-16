#include "SensorUnitManager.h"
void SensorUnitManager::sendToSu(Packet packet, int suNum) {
  esp_err_t result =
      esp_now_send(m_suMac[suNum], (uint8_t *)&packet, sizeof(packet));
}

void sensUnitManagerSendCB(const uint8_t *mac, esp_now_send_status_t status) {}

void sensUnitManagerRecvCB(const esp_now_recv_info_t *recvInfo,
                           const uint8_t *data, int dataLen) {}
