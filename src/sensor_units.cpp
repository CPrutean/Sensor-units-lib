#include "sensor_units.h"
#include "esp_now.h"
#include "sensors_src/gps_sensor.h"
#include "sensors_src/motion_sensor.h"
#include "sensors_src/temperature_sensor.h"

/*
@breif clears the EEPROM cache memory
*/
void clearEEPROM() {
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("Failed to initialize EEPROM for clearing.");
    return;
  }
  Serial.println("Clearing EEPROM...");
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0xFF); // Write 0xFF (erased state) to each byte
  }
  if (EEPROM.commit()) {
    Serial.println("EEPROM successfully cleared.");
  } else {
    Serial.println("Failed to commit EEPROM clear.");
  }
  EEPROM.end();
}

/*
@breif writes a string to EEPROM cache which ends the string in a sentinel value
@param name: the string to write to EEPROM cache
@param *msg: default message parameter for error handling
*/
void writeToEEPROM(const char *name, def_message_struct *msg) {
  if (!EEPROM.begin(EEPROM_SIZE)) {
    snprintf(msg->message, sizeof(msg->message) - 1, "%s",
             "FAILED TO BEGIN EEPROM");
  }
  int i;
  int len = strlen(name);
  for (i = 0; i < len; i++) {
    EEPROM.put(i, name[i]);
  }
  EEPROM.put(i + 1, 0xff);
  if (!EEPROM.commit()) {
    snprintf(msg->message, sizeof(msg->message), "%s",
             "FAILED TO COMMIT EEPROM");
  }
  EEPROM.end();
}

/*
@breif reads a string from EEPROM and terminates once it reaches the sentinel
value
@param nameDest: the string buffer to write the name into
@param destSize: the size of the string buffer
@param *msg: default message pointer for error handling if something goes wrong
*/
bool readFromEEPROM(char *nameDest, int destSize, def_message_struct *msg) {
  if (msg == NULL) {
    return false;
  }
  if (!EEPROM.begin(EEPROM_SIZE)) {
    msg->message[0] = '\0';
    snprintf(nameDest, destSize, "%s", "Failed to being EEPROM");
    return false;
  } else if (EEPROM.read(0) == 0xff) {
    nameDest[0] = '\0';
    snprintf(nameDest, destSize, "%s", "EEPROM was empty");
    return false;
  }
  int i = 0;
  while (i < destSize && EEPROM.read(i) != 0xff) {
    nameDest[i] = EEPROM.read(i);
    i++;
  }
  EEPROM.end();
  return true;
}

sensor_unit::sensor_unit(sensor_type *modules, uint8_t numSensors, char *nameIn,
                         temperature_sensor *tempSensorIn, gps_sensor *gpsIn,
                         motion_sensor *motionIn) {
  this->motionSensor = (motionIn == nullptr) ? motion_sensor() : *motionIn;
  this->gpsSensor = (gpsIn == nullptr) ? gps_sensor() : *gpsIn;
  this->tempSensor =
      (tempSensorIn == nullptr) ? temperature_sensor() : *tempSensorIn;
  int i;
  for (i = 0; i < numSensors; i++) {
    this->modules[i] = modules[i];
  }
}

void sensor_unit::initESP_NOW(uint8_t *cuAddrIn, const char *PMK_KEY,
                              const char *LMK_KEY) {
  // TODO implement initializing ESPNOW with proper encryption.
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
#ifdef DEBUG
    Serial.println("ESP-NOW failed to init exiting");
#endif
    return;
  }
  memcpy(this->cuPeerInf.peer_addr, cuAddrIn, 6);
  this->cuPeerInf.channel = 0;
  esp_now_set_pmk((uint8_t *)PMK_KEY);
  for (uint8_t i = 0; i < 16; i++) {
    this->cuPeerInf.lmk[i] = (uint8_t)*(LMK_KEY + i);
  }
  this->cuPeerInf.encrypt = true;
  if (esp_now_add_peer(&this->cuPeerInf) != ESP_OK) {
#ifdef DEBUG
    Serial.println("Failed to add peer");
#endif
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(onReceiveCBSU));
  esp_now_register_send_cb(esp_now_send_cb_t(onSendCBSU));
}

void sensor_unit::sendMessage(def_message_struct msg) {
  esp_err_t status = esp_now_send((uint8_t *)&msg, this->cuPeerInf.peer_addr,
                                  sizeof(def_message_struct));
#ifdef DEBUG
  if (status != ESP_OK) {
    Serial.println("Message failed to send");
  } else {
    Serial.println("Message sent successfully");
  }
#endif
}

void sensor_unit::handleMsg(def_message_struct msgIn) {
  def_message_struct msgOut;
  memcpy(&msgOut, &msgIn, sizeof(def_message_struct));
  switch (msgIn.sensor_req) {
  case (TEMP_AND_HUMID):
    if (!this->tempSensor.isValidInstance()) {
      snprintf(msgOut.message, sizeof(msgOut.message) - 1, "%s",
               "INVALID SENSOR PASSED");
      msgOut.type = STRING_T;
      this->sendMessage(msgOut);
      return;
    }
    if (msgIn.command_ind == 0) {
      msgOut.values[0] = this->tempSensor.pullTemp();
      msgOut.numValues = 1;
    } else if (msgIn.command_ind == 1) {
      msgOut.values[0] = this->tempSensor.pullHumid();
      msgOut.numValues = 1;
    } else if (msgIn.command_ind == 2) {
      snprintf(msgOut.message, sizeof(msgOut.message) - 1, "%c",
               this->tempSensor.pullPref());
      msgOut.type = STRING_T;
    } else if (msgIn.command_ind == 3) {
      this->tempSensor.pushPref(msgIn.message[0]);
    }
    break;

  case (GPS):
    if (!this->gpsSensor.isValidInstance()) {
      snprintf(msgOut.message, sizeof(msgOut.message) - 1, "%s",
               "INVALID SENSOR PASSED");
      msgOut.type = STRING_T;
      this->sendMessage(msgOut);
    }

    if (msgIn.command_ind == 0) {
      msgOut.values[0] = this->gpsSensor.pullLat();
      msgOut.values[1] = this->gpsSensor.pullLong();
      msgOut.numValues = 2;
    }
    break;

  case (MOTION_SENSOR):
    if (!this->motionSensor.isValidInstance()) {
      snprintf(msgOut.message, sizeof(msgOut.message) - 1, "%s",
               "INVALID SENSOR PASSED");
      msgOut.type = STRING_T;
      this->sendMessage(msgOut);
    }

    if (msgIn.command_ind == 0) {
      msgOut.values[0] = this->motionSensor.pullMotion();
      msgOut.numValues = 1;
    }
    break;

  default:
    sensor_unit_status status = ONLINE;
    if (msgIn.command_ind == 0) {
      for (uint8_t i = 0; i < this->moduleCount; i++) {
        if (this->modules[i] == TEMP_AND_HUMID &&
            !this->tempSensor.isValidInstance() &&
            !this->tempSensor.isFunctioning()) {
          status = ERROR;
          snprintf(msgOut.message, strlen(msgOut.message) - sizeof(msgOut) - 1,
                   "%s", "TEMP_SENS_ERR|");
          msgOut.type = STRING_T;
        } else if (this->modules[i] == GPS &&
                   !this->gpsSensor.isValidInstance() &&
                   !this->gpsSensor.isFunctioning()) {
          status = ERROR;
          snprintf(msgOut.message, strlen(msgOut.message) - sizeof(msgOut) - 1,
                   "%s", "GPS_SENS_ERR|");
          msgOut.type = STRING_T;
          msgOut.type = STRING_T;
        } else if (this->modules[i] == MOTION_SENSOR &&
                   !this->motionSensor.isValidInstance() &&
                   !this->motionSensor.isFunctioning()) {
          status = ERROR;
          snprintf(msgOut.message, strlen(msgOut.message) - sizeof(msgOut) - 1,
                   "%s", "MOTION_SENS_ERR|");
          msgOut.type = STRING_T;
        }

        msgOut.values[0] = static_cast<double>(status);
        msgOut.numValues = 1;
      }
    } else if (msgIn.command_ind == 1) {
      int lengthOfValues = sizeof(msgOut.values) / sizeof(msgOut.values[0]);
      for (uint8_t i = 0; i < this->moduleCount && i < lengthOfValues; i++) {
        msgOut.values[i] = static_cast<double>(this->modules[i]);
        msgOut.numValues++;
      }
    } else if (msgIn.command_ind == 2) {
      msgOut.type = STRING_T;
      if (!readFromEEPROM(msgOut.message, sizeof(msgOut.message) - 1,
                          &msgOut)) {
        msgOut.message[0] = '\0';
        snprintf(msgOut.message, sizeof(msgOut.message) - 1, "%s", this->name);
      }
    } else if (msgIn.command_ind == 3) {
      msgOut.type = STRING_T;
      writeToEEPROM(msgIn.message, &msgOut);
      if (msgOut.message[0] == '\0') {
        snprintf(msgOut.message, sizeof(msgOut.message) - 1, "%s", "SUCCESS");
      } else {
        snprintf(msgOut.message, sizeof(msgOut.message) - 1, "%s", "FAILED");
      }
    }
    break;
  };
}
