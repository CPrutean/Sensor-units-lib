#include "sensor_units.h"

// Py string word seperator
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
/* @breif: communicates with the directly plugged in server using UART and the UART buffer
 * @param *buffer: the buffer being printed
 */
void stageForReturn(char *buffer) { Serial.println(buffer); }

/* @breif: returns the amount of sensor_units available
 * @return: the number of sensor_units in the class
 */
uint8_t communication_unit::getSuCount() const {
  return m_numOfSU;
}


communication_unit::communication_unit(uint8_t numOfSu, uint8_t** suMac, char** suNames, const char *PMK_KEY_IN, const char *LMK_KEY_IN) {
  this->m_numOfSU = numOfSu;
  int i = 0;
  int j = 0;
  //create deep copy for ease of use later
  for (i = 0; i < numOfSu; i++) {
    m_names[i] = suNames[i];
    for (j = 0; j < 6; j++) {
      m_suMac[i][j] = suMac[i][j];
    }
  }
  this->initESP_NOW(PMK_KEY_IN, LMK_KEY_IN);
}


/* @breif: Sends a message using ESP-NOW, sends it to the message confirmation
 * system and returns the corresponding message ID of the message that was sent
 * @param msgOut: the default message struct being sent out.
 * @param SUIND: the index corresponding to the sensor unit being assigned
 * @return: Returns the message ID corresponding to the message sent out.
 */
#ifdef DEBUG
unsigned long int communication_unit::sendMessage(def_message_struct& msgOut, int SUIND) {
  msgOut.msgID = m_msgID++;
  if (SUIND > this->m_numOfSU || SUIND < 0) {
    stageForReturn("INVALID SU IND");
    return -1;
  }

  esp_err_t status = esp_now_send(m_suPeerInf[SUIND].peer_addr, (uint8_t *)&msgOut, sizeof(def_message_struct));
  this->acknowledgementQueue->addToWaiting(msgOut, SUIND); 


  if (status != ESP_OK) {
    this->acknowledgementQueue->moveToFailed(msgOut.msgID);
  }
  return m_msgID - 1;
}
#else //For testing will only return the index of the sensor unit
unsigned long communication_unit::sendMessage(def_message_struct msgOut, int SUIND) {
  return SUIND;
}
#endif


/* breif: initializes ESP-NOW with valid encryption and sensor unit addresses.
 * param suAddr: a 2D array consisting of individual mac addresses for sensor
 * units param numOfSU: the amount of sensor units being added param PMK_KEY_IN:
 * a 16 byte PMK key used for setting up encryption param LMK_KEY_IN: a 16 byte
 * array used for initializing encryption
 *
 * */
void communication_unit::initESP_NOW(const char *PMK_KEY_IN, const char *LMK_KEY_IN) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
#ifdef DEBUG
    Serial.println("ESP-NOW failed to init exiting");
#endif
    return;
  }

  esp_now_set_pmk((uint8_t *)PMK_KEY_IN);
  for (uint8_t i = 0; i < m_numOfSU; i++) {
    memcpy(m_suPeerInf, *(m_suMac + i), 6);
    for (uint8_t j = 0; j < 16; j++) {
      m_suPeerInf[i].lmk[j] = *(LMK_KEY_IN + i);
    }
    m_suPeerInf[i].encrypt = true;
    m_suPeerInf[i].channel = 0;
    if (esp_now_add_peer(m_suPeerInf) != ESP_OK) {
#ifdef DEBUG
      Serial.println("Failed to add a peer");
#endif
    }
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(onReceiveCBCU));
  esp_now_register_send_cb(esp_now_send_cb_t(onSendCBCU));
}
/* breif: handles messages coming in from sensor units and returns the
 * information back to the server that the communication unit is attached to via
 * UART param msgIn: The message that is being returned back to the server
 *
 * */
char strSeper[] = {'|', '\0'};
char bufferSeper[]{'\n', '\0'};

void communication_unit::handleMsg(def_message_struct msgIn, char* printbuffer) {
  char returnBuffer[1000];
  returnBuffer[0] = '\0';
  snprintf(returnBuffer, sizeof(returnBuffer) - 1, "%s",sensors[msgIn.sensor_req].responses[msgIn.command_ind]);

  snprintf(returnBuffer, sizeof(returnBuffer) - strlen(returnBuffer) - 1, "%s",strSeper);
  if (msgIn.type == STRING_T) {
    snprintf(returnBuffer, sizeof(returnBuffer) - strlen(returnBuffer) - 1, "%s", msgIn.message);
  } else if (msgIn.type == DOUBLE_T && msgIn.sensor_req != BASE_SENS_UNIT &&
             msgIn.command_ind != 0 && msgIn.command_ind != 1) {
    for (uint8_t i = 0; i < msgIn.numValues; i++) {
      snprintf(returnBuffer, sizeof(returnBuffer) - strlen(returnBuffer) - 1, "%f", msgIn.values[i]);
      snprintf(returnBuffer, sizeof(returnBuffer) - strlen(returnBuffer) - 1, "%s", strSeper);
    }
  } else if (msgIn.sensor_req == BASE_SENS_UNIT && msgIn.command_ind == 0) { // Pull status
    snprintf(returnBuffer, sizeof(returnBuffer) - strlen(returnBuffer) - 1, "%s", status_strings[static_cast<int>(msgIn.values[0])]);
  } else if (msgIn.sensor_req == BASE_SENS_UNIT && msgIn.command_ind == 1) { // Pull sens_units
    for (uint8_t i = 0; i < msgIn.numValues; i++) {
      snprintf(returnBuffer, sizeof(returnBuffer) - strlen(returnBuffer) - 1, "%s", sens_unit_strings[static_cast<int>(msgIn.values[i])]);
      snprintf(returnBuffer, sizeof(returnBuffer) - strlen(returnBuffer) - 1, "%s", strSeper);
    }
  } else {
    snprintf(returnBuffer, sizeof(returnBuffer) - strlen(returnBuffer) - 1, "%s", strSeper);
    snprintf(returnBuffer, sizeof(returnBuffer) - strlen(returnBuffer) - 1, "%s", "INVALID TYPE PASSED");
  }
  snprintf(returnBuffer, sizeof(returnBuffer) - 1, "%s", sensors[msgIn.sensor_req].responses[msgIn.command_ind]);

  snprintf(returnBuffer, sizeof(returnBuffer) - strlen(returnBuffer) - 1, "%s", "MSG_ID:");
  snprintf(returnBuffer, sizeof(returnBuffer) - strlen(returnBuffer) - 1, "%ld", msgIn.msgID);
  #ifdef DEBUG
  if (printbuffer != nullptr) { //Only use for testing
    snprintf(printbuffer, sizeof(char)*1000, "%s", returnBuffer);
  }
  #endif

  stageForReturn(returnBuffer);
}
/* breif: handles requests from the server and sends the requests to the
 * corresponding sensor_unit special use case when initializing the system where
 * the communication unit will return back all the sensor units and sensors
 * available in the current firmware version
 *
 * param buffer: the string buffer used for the request
 * param sizeOfBuffer: the length of the buffer being read from
 */

void communication_unit::handleServerRequest(char *buffer, int sizeOfBuffer, def_message_struct *msgPtr) {
  if (sizeOfBuffer < 3) {
    stageForReturn("INVALID REQUEST");
    return;
  }
  if (sizeOfBuffer > 4 && strncmp(buffer, "INIT", 4) == 0) {
    def_message_struct msg;
    msg.sensor_req = BASE_SENS_UNIT;
    for (uint8_t i = 0; i < this->m_numOfSU; i++) { // Only send the first 3 BASE_SENS_UNIT commands since we are
                // just initializing the communication unit
      for (uint8_t j = 0; j < 3; j++) {
        msg.command_ind = j;
        unsigned long id = sendMessage(msg, i);
        char msgIdBuf[30] = "MSGID:";
        snprintf(msgIdBuf, sizeof(msgIdBuf) - strlen(msgIdBuf) - 1, "%ld", id);
        stageForReturn(msgIdBuf);
      }
    }
  }

  int SUIND = buffer[0];
  sensor_type sensor = static_cast<sensor_type>(buffer[1]);
  int cmdInd = buffer[2];

  def_message_struct msgOut;
  msgOut.command_ind = cmdInd;
  msgOut.sensor_req = sensor;
  if (sizeOfBuffer > 3) {
    snprintf(msgOut.message, sizeof(msgOut.message), "%s", buffer + 3);
  }
  unsigned long msgId = this->sendMessage(msgOut, SUIND);
  #ifdef DEBUG //For debugging puproses only you may need access to the message that was generated
  if (msgPtr != nullptr) {
    msgPtr = &msgOut;
  }
   #endif
  char msgIdBuf[30] = "MSGID:";
  snprintf(msgIdBuf, sizeof(msgIdBuf) - strlen(msgIdBuf) - 1, "%ld", msgId);
  stageForReturn(msgIdBuf);
}
