#include "sensor_units.h"

/*
@breif: checks if the mac address is all 0 to determine if its a valid or
invalid address
@param mac[6]: the mac address passed
*/
static inline bool is_zero_mac(const uint8_t mac[6]) {
  for (int i = 0; i < 6; i++) {
    if (mac[i] != 0) {
      return false;
    }
  }
  return true;
}

void onReceiveCBSU(uint8_t *macAddr, uint8_t *data, int size) {}

void onReceiveCBCU(uint8_t *macAddr, uint8_t *data, int size) {}

void onSendCBSU(uint8_t *macAddr, esp_now_send_status_t status) {}

void onSendCBCU(uint8_t *macAddr, esp_now_send_status_t status) {}
