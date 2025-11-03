#include "TinyGPS++.h"
#include "esp_now.h"
#include "sensor_units.h"

// Py string word seperator
char strSeper[] = {'|', '\0'};
char bufferSeper[]{'\n', '\0'};

void stageForReturn(char *str, int buffSize) {
  char tempStr[MAX_MSG_LENGTH];
  tempStr[0] = '\0';
  strncpy(tempStr, str, sizeof(tempStr));
  strncat(tempStr, piBufferSeper, sizeof(tempStr) - strlen(tempStr) - 1);
  Serial.print(tempStr);
}

/*
@brief: basic substring function to write a start of a function
@param source: the source string which we are taking from
@param start: where we start copying from
@param len: the length of how much we are copying
@param dest: the destination for where we are copying into
@param bufferLen: the length of the buffer which we are copying into
@return: returns -1 if it failed to copy into the buffer and 0 if it succesfully
copied into the buffer
*/
int substring(const char *source, int start, int len, char *dest,
              int bufferLen) {
  if (dest == NULL || len > bufferLen) {
    return -1;
  }
  strncpy(dest, source + start, len);
  dest[len] = '\0';
  return 0;
}

void communication_unit::sendMessage(def_message_struct msgOut, int SUIND) {
  msgOut.msgID = this->msgID++;
  if (SUIND > this->numOfSU || SUIND < 0) {
#ifdef DEBUG
    Serial.println("INVALID SU IND");
#endif
    return;
  }
  esp_err_t status =
      esp_now_send(this->suPeerInf[SUIND].peer_addr, (uint8_t *)&msgOut,
                   sizeof(def_message_struct));
  this->acknowledgementQueue.addToWaiting(msgOut,
                                          this->suPeerInf[SUIND].peer_addr);

  if (status != ESP_OK) {
    this->acknowledgementQueue.moveToFailed(msgOut.msgID);
  }
}

communication_unit::communication_unit() {
  this->msgID = 0;
  this->acknowledgementQueue = messageAcknowledge();
  this->messageQueue = msg_queue();
}

void communication_unit::initESP_NOW(uint8_t **suAddr, uint8_t numOfSU,
                                     const char *PMK_KEY_IN,
                                     const char *LMK_KEY_IN) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
#ifdef DEBUG
    Serial.println("ESP-NOW failed to init exiting");
#endif
    return;
  }

  esp_now_set_pmk((uint8_t *)PMK_KEY_IN);
  for (uint8_t i = 0; i < numOfSU; i++) {
    memcpy(this->suPeerInf, *(suAddr + i), 6);
    for (uint8_t j = 0; j < 16; j++) {
      this->suPeerInf[i].lmk[j] = *(LMK_KEY_IN + i);
    }
    this->suPeerInf[i].encrypt = true;
    this->suPeerInf[i].channel = 0;
    if (esp_now_add_peer(this->suPeerInf) != ESP_OK) {
#ifdef DEBUG
      Serial.println("Failed to add a peer");
#endif
    }
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(onReceiveCBCU));
  esp_now_register_send_cb(esp_now_send_cb_t(onSendCBCU));
}

// TODO implement proper message handling for sensor units and server side
// requests
void communication_unit::handleMsg(def_message_struct msgIn) {
  char returnBuffer[1000];
  snprintf(returnBuffer, sizeof(returnBuffer) - 1, "%s",
           sensors[msgIn.sensor_req].responses[msgIn.command_ind]);
  if (msgIn.type == STRING_T) {

  } else if (msgIn.type == DOUBLE_T) {

  } else {
    snprintf(returnBuffer, sizeof(returnBuffer) - strlen(returnBuffer) - 1,
             "%s", strSeper);
    snprintf(returnBuffer, sizeof(returnBuffer) - strlen(returnBuffer) - 1,
             "%s", "INVALID TYPE PASSED");
  }
}

void communication_unit::handleServerRequest(char *buffer, int sizeOfBuffer) {}
