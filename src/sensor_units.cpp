#include "sensor_units.h"
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
    snprintf(msg->message, sizeof(msg->message), "%s",
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
    snprintf(msg->message, sizeof(msg->message), "%s",
             "Failed to being EEPROM");
    return false;
  } else if (EEPROM.read(0) == 0xff) {
    msg->message[0] = '\0';
    snprintf(msg->message, sizeof(msg->message), "%s", "EEPROM was empty");
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

/*
@breif determines the status of each sensor available in the sensor unit in
externally defined sens_unit_ptr
@param *msg: the default message for error handling reasons
*/
STATUS determineStatus() {}

/*
@breif: returns the sensor units available in the externally defined
sens_unit_ptr
@param *msg: default message struct to return error messages
@return: returns whether or not the command exited succesfully
*/
bool returnSensUnits(def_message_struct *msg) {
  if (sens_unit_ptr->moduleCount == 0 || sens_unit_ptr->modules == NULL) {
    return false;
  }
  int i;
  for (i = 0; i < sens_unit_ptr->moduleCount &&
              i < (sizeof(msg->values) / sizeof(msg->values[0]));
       i++) {
    msg->values[i] = (int)sens_unit_ptr->modules[i];
    msg->numValues++;
  }
  return true;
}

/*
@breif: handles requests made to different sensors and what values to return
based off the index of the command passed and the command passed
@param sensor: the sensor which corresponds to what sensor to pull values from
if noe is passed assume default global sensor unit commands
@param *msg: default message struct to write error codes to
@param ind: index of the command passed
@param cmd_passed: the command thats passed mainly only used in the case of a
new name being assigned to the sensor unit
*/
void handleSensorRequests(sensor_type sensor, def_message_struct *msg, int ind,
                          char *cmd_passed) {
  if (sens_unit_ptr == nullptr) {
#ifdef DEBUG
    Serial.println("sens_unit_ptr was never initialized");
#endif
    return;
  }
  // TODO finish logic for commands
  def_message_struct response;
  response.command_ind = msg->command_ind;
  response.sensor_req = msg->sensor_req;
  response.msgID = msg->msgID;
  switch (msg->sensor_req) {
  case (TEMP_AND_HUMID):
    if (sens_unit_ptr->temperatureSensor == nullptr) {
      snprintf(response.message, sizeof(response.message), "%s",
               "TEMP SENSOR NOT AVLBL");
    } else if (msg->command_ind == 0) {
      response.values = sens_unit_ptr->temperatureSensor->pullTemp();
      response.numValues = 1;
    } else if (msg->command_ind == 1) {
      response.values = sens_unit_ptr->temperatureSensor->pullHumid();
      response.numValues = 1;
    } else if (msg->command_ind == 2) {
      response.values[0] = static_cast<char>(
          static_cast<int>(sens_unit_ptr->temperatureSensor->pullPref));
    } else if (msg->command_ind == 3) {
      sens_unit_ptr->temperatureSensor->pushPref(
          static_cast<char>(msg->values[0]));
    } else {
      snprintf(response.message, sizeof(response.message), "%s",
               "INVALID COMMAND IND");
    }
    break;
  case (GPS):
    if (sens_unit_ptr->gps == nullptr) {
      snprintf(response.message, sizeof(response.message), "%s",
               "GPS SENSOR NOT AVLBL");
    } else if (msg->command_ind) {
      response.values[0] = sens_unit_ptr->gps->pullLat();
      response.values[1] = sens_unit_ptr->gps->pullLong();
      response.numValues = 2;
    } else {
      snprintf(response.message, sizeof(response.message), "%s",
               "INVALID IND PASSED");
    }
    break;
  case (MOTION_SENSOR):
    if (sens_unit_ptr->motionSensor == nullptr) {
      snprintf(response.message, sizeof(response.message), "%s",
               "MOTION_SENS NOT AVLBL");
    } else if (ind == 0) {
      response.values[0] = sens_unit_ptr->motionSensor->pullMotion();
    } else {
      snprintf(response.message, sizeof(response.message), "%s",
               "INVALID IND PASSED");
    }
    break;
  case (BASE_SENS_UNIT):
    if (ind == 0) {
      response.values[0] = determineStatus();
    } else if (ind == 1) {
      int i;
      response.numValues = 0;
      for (i = 0; i < sens_unit_ptr->moduleCount; i++) {
        response.values[i] = sens_unit_ptr->modules[i];
        response.numValues++;
      }
    } else if (ind == 2) {
      snprintf(response.message, sizeof(response.message), "%s",
               sens_unit_ptr->name);
    } else if (ind == 3) {
      sens_unit_ptr->name[0] = '\0';
      snprintf(sens_unit_ptr->name, sizeof(sens_unit_ptr->name), "%s",
               msg->message);
      writeToEEPROM(msg->message, &response);
    } else {
      snprintf(response.message, sizeof(response.message), "%s",
               "INVALID IND PASSED");
    }
    break;
  default:
    break;
  }
}
