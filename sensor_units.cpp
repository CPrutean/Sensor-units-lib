#include "sensor_units.h"

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
void writeToEEPROM(const char* name, def_message_struct* msg) {
    if (!EEPROM.begin(EEPROM_SIZE)) {
        msg->strlen += snprintf(msg->message, MAX_MSG_LENGTH, "%s", "FAILED TO INITIATE EEPROM");
    }
    int i;
    int len = strlen(name);
    for (i = 0; i < len; i++) {
       EEPROM.put(i, name[i]); 
    }
    EEPROM.put(i+1, 0xff);
    if (!EEPROM.commit()) {
        msg->strlen += snprintf(msg->message, MAX_MSG_LENGTH, "%s", "FAILED TO COMMIT TO EEPROM");
    }
    EEPROM.end();
}

/*
@breif reads a string from EEPROM and terminates once it reaches the sentinel value
@param nameDest: the string buffer to write the name into
@param destSize: the size of the string buffer
@param *msg: default message pointer for error handling if something goes wrong
*/
bool readFromEEPROM(char* nameDest, int destSize, def_message_struct *msg) {
    if (msg == NULL) {
        return false;
    }
    if (!EEPROM.begin(EEPROM_SIZE)) {
        msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", "FAILED TO INITIATE EEPROM");
        return false;
    } else if (EEPROM.read(0) == 0xff) {
        msg->message[0] = '\0';
        msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", "EEPROM name was empty init a name");
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
@breif determines the status of each sensor available in the sensor unit in externally defined sens_unit_ptr
@param *msg: the default message for error handling reasons
*/
void determineStatus(def_message_struct *msg) {
    if (sens_unit_ptr == nullptr) {
        msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", sens_unit_response[0]);
        msg->strlen += snprintf(msg->message+msg->strlen, MAX_MSG_LENGTH, "%s", ":sens_unit_ptr was never intialized");
        msg->values[0] = (float)ERROR;
        return;
    }

    msg->values[0] = (float)ONLINE; 
    strncpy(msg->message, sens_unit_response[0], MAX_MSG_LENGTH); 

    for (int i = 0; i < sens_unit_ptr->moduleCount; i++) { // Assuming NULL_SENSOR_TYPE terminates the array
        sensor_type current_sensor = sens_unit_ptr->modules[i];
        
        if (current_sensor == TEMP_AND_HUMID && (sens_unit_ptr->dht_sensor == nullptr ||sens_unit_ptr->dht_sensor->readTemperature() == NULL)) {
            msg->values[0] = (float)ERROR;
            strncat(msg->message, " DHT_FAIL", MAX_MSG_LENGTH - strlen(msg->message) - 1);
        } 
        else if (current_sensor == GPS && (sens_unit_ptr->gpsSerial == nullptr || sens_unit_ptr->gps == nullptr || !sens_unit_ptr->gpsSerial->available() || !sens_unit_ptr->gps->location.isValid())) {
            msg->values[0] = (float)ERROR;
            strncat(msg->message, " GPS_FAIL", MAX_MSG_LENGTH - strlen(msg->message) - 1);
        }
        msg->numValues = 1;
    }
}


/*
@breif: returns the sensor units available in the externally defined sens_unit_ptr
@param *msg: default message struct to return error messages
@return: returns whether or not the command exited succesfully
*/
bool returnSensUnits(def_message_struct* msg) {
    if (sens_unit_ptr->moduleCount == 0 || sens_unit_ptr->modules == NULL) {
        return false;
    }
    int i;
    for (i = 0; i < sens_unit_ptr->moduleCount && i < (sizeof(msg->values)/sizeof(msg->values[0])); i++) {
        msg->values[i] = (int)sens_unit_ptr->modules[i];
        msg->numValues++;
    }
    return true;
}


/*
@breif: handles requests made to different sensors and what values to return based off the index of the command passed and the command passed
@param sensor: the sensor which corresponds to what sensor to pull values from if none is passed assume default global sensor unit commands
@param *msg: default message struct to write error codes to
@param ind: index of the command passed
@param cmd_passed: the command thats passed mainly only used in the case of a new name being assigned to the sensor unit
*/
void handleSensorRequests(sensor_type sensor, def_message_struct *msg, int ind, char* cmd_passed) {
    switch (sensor) {
        case TEMP_AND_HUMID:
            if (sens_unit_ptr->dht_sensor != nullptr && ind == 0) {
                msg->values[0] = sens_unit_ptr->dht_sensor->readTemperature();
                 msg->value[0] = '\0';
                snprintf(msg->value, sizeof(msg->value), "%s", sensors[sensor].values[0]);
            } else if (sens_unit_ptr->dht_sensor != nullptr && ind == 1) {
                msg->values[0] = sens_unit_ptr->dht_sensor->readHumidity();
                msg->value[0] = '\0';
                snprintf(msg->value, sizeof(msg->value), "%s", sensors[sensor].values[1]);
            } else {
                msg->message[0] = '\0';
                msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", "UNABLE TO FIND READING");
            }
            msg->numValues = 1;
            break;
        case GPS:
            msg->numValues = 2;
            if (sens_unit_ptr->gpsSerial != nullptr && sens_unit_ptr->gps != nullptr && ind == 0) {
                msg->values[0] = sens_unit_ptr->gps->location.lat();
                msg->values[1] = sens_unit_ptr->gps->location.lng();
                msg->value[0] = '\0';
                snprintf(msg->value, sizeof(msg->value), "%s", sensors[sensor].values[0]);
                msg->numValues = 2;
            } else {
                msg->message[0] = '\0';
                msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", "UNABLE TO FIND READING");
                msg->numValues = 0;
            }
            break;
        case (BASE_SENS_UNIT):
            if (ind == 0) {
                determineStatus(msg);
                msg->value[0] = '\0';
                snprintf(msg->value, sizeof(msg->value), "%s", sensors[sensor].values[0]);
            } else if (ind == 1) {
                returnSensUnits(msg);
                msg->value[0] = '\0';
                snprintf(msg->value, sizeof(msg->value), "%s", sensors[sensor].values[0]);
            } else if (ind == 2) {
                msg->message[0] = '\0';
                if (sens_unit_ptr->name[0] != '\0') {
                    msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", sens_unit_response[2]);
                    strncat(msg->message, "|", MAX_MSG_LENGTH-msg->strlen-1);
                    msg->strlen++;
                    strncat(msg->message, sens_unit_ptr->name, MAX_MSG_LENGTH-(msg->strlen-1));
                    msg->strlen+=strlen(sens_unit_ptr->name);
                } else {
                    msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", "NAME WAS NULL PUSH A NAME TO THIS DEVICE");
                }
                msg->value[0] = '\0';
                snprintf(msg->value, sizeof(msg->value), "%s", sensors[sensor].values[0]);
            } else if (ind == 3) {
                sens_unit_ptr->name[0] = '\0';
                int len = strlen(cmd_passed);
                int i = 0;
                while (cmd_passed[i] != '|') {
                    i++;
                }
                if (i < len) {
                    i++;
                    sens_unit_ptr->name[0] = '\0';
                    substring(cmd_passed, i, (len-1-i), sens_unit_ptr->name, MAX_NAME_LEN);
                    msg->message[0] = '\0';
                    msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", sens_unit_response[3]);
                    strncat(msg->message, "|", MAX_MSG_LENGTH-msg->strlen);
                    msg->strlen += strlen("|");
                    strncat(msg->message, sens_unit_ptr->name, MAX_MSG_LENGTH-msg->strlen);
                    msg->strlen = strlen(msg->message);
                    writeToEEPROM(sens_unit_ptr->name, msg);
                } else {
                    msg->message[0] = '\0';
                    msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", "Name failed to set due to invalid command being sent");
                }
                msg->value[0] = '\0';
                snprintf(msg->value, sizeof(msg->value), "%s", sensors[sensor].values[0]);
            } else {
                msg->message[0] = '\0';
                msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", "INVALID INDEX PASSED");
            }
            break;
        case (MOTION_SENSOR):
            if (sens_unit_ptr != nullptr) {
                int i;
                bool found = false;
                for (i = 0; i < sens_unit_ptr->moduleCount; i++) {
                    if (sens_unit_ptr->modules[i] == MOTION_SENSOR) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    msg->message[0] = '\0';
                    msg->strlen = snprintf(msg->message, sizeof(def_message_struct), "%s","Unable to find motion sensor unit in available modules"); 
                    return;
                }
                
            } else if (sens_unit_ptr->motion == nullptr) {
                msg->message[0] = '\0';
                msg->strlen = snprintf(msg->message, sizeof(def_message_struct), "%s", "Motion sensor pointer was never initialized");    
                return;
            } else if (sens_unit_ptr->motion->count() == 1) {
                msg->message[0] = '\0';
                msg->strlen = snprintf(msg->message, sizeof(def_message_struct), "%s", "Initialize only one motion sensor per PIR counter");    
            }

            //Motion sensor only has one value so in this case we can only pull onve value
            msg->values[0] = sens_unit_ptr->motion->read();
            break;
        default:
            msg->message[0] = '\0';
            msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", "INVALID SENSOR FOUND");
            break;
    }
}


/*
brief: handle messages passed to it and commands
param cmd_passed: the string commmand passed to it
param *response: the default message struct to modify
*/
void handleRequestSU(def_message_struct msgRecv, def_message_struct *response) {
    memset(response, 0, sizeof(def_message_struct));
    response->message[0] = '\0';
    response->value[0] = '\0';
    if (sens_unit_ptr == nullptr) {
        response->strlen = snprintf(response->message, MAX_MSG_LENGTH, "%s", "sens_unit_ptr was never initialized");
        return;
    }
    int i;
    int j = 0;
    
    bool completed = false;
    for (i = 0; i < NUM_OF_SENSORS; i++) {
        while (sensors[i].commands[j]!=NULL) {
            if (strncmp(sensors[i].commands[j], msgRecv.message, strlen(sensors[i].commands[j])) == 0) {
                response->strlen = snprintf(response->message, MAX_MSG_LENGTH, "%s", sensors[i].responses[j]);
                handleSensorRequests(sensors[i].sensor, response, j, msgRecv.message);
                completed = true;
                break;
            }
            j++;
        }
        if (completed) {
            break;
        }
        j = 0;
    }
    if (!completed) {
        memset(response, 0, sizeof(response));
        response->message[0] = '\0';
        response->strlen = snprintf(response->message, MAX_MSG_LENGTH, "%s", "FAILED TO FIND REQUESTED VALUES");
    }
    response->msgID = msgRecv.msgID;
    sendMessage(sens_unit_ptr->CU_ADDR, (uint8_t*)response, sizeof(*response));
}