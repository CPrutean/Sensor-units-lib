#include "sensor_units.h"

//Writes all values passed to EEPROM
//To prevent excessive read and write limits writings will only be staged to commit when youre pushing multiple commands
//Addresses will be stored after the available index for information

//Clears EEPROM cache
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

//Writes values collected to EEPROM flash memory
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


//Reads the EEPROM values stored
bool readFromEEPROM(char* nameDest, int destSize, def_message_struct *msg) {
    if (msg == NULL) {
        return false;
    }
    if (!EEPROM.begin(EEPROM_SIZE)) {
        msg->strlen += snprintf(msg->message, MAX_MSG_LENGTH, "%s", "FAILED TO INITIATE EEPROM");
        return false;
    } else if (EEPROM.read(0) == 0xff) {
        msg->message[0] = '\0';
        msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", "EEPROM name was empty init a name");
    }
    int i = 0;
    while (i < destSize && EEPROM.read(i) != 0xff) {
        nameDest[i] = EEPROM.read(i);
        i++;
    }
    EEPROM.end();
    return true;
}

//Determines the status of EEPROM sensors to return
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
        else if (current_sensor == GPS && (sens_unit_ptr->gpsSerial == nullptr || sens_unit_ptr->gps == nullptr || !sens_unit_ptr->gpsSerial->available())) {
            msg->values[0] = (float)ERROR;
            strncat(msg->message, " GPS_FAIL", MAX_MSG_LENGTH - strlen(msg->message) - 1);
        }
        msg->numValues = 1;
    }
}

bool returnSensUnits(def_message_struct* msg) {
    if (sens_unit_ptr->moduleCount = 0 || sens_unit_ptr->modules == NULL) {
        return false;
    }
    int i;
    for (i = 0; i < sens_unit_ptr->moduleCount && i < sizeof(msg->values)/sizeof(msg->values[0]); i++) {
        msg->values[i] = (float)sens_unit_ptr->modules[i];
    }
    return true;
}

//For the sake of storing something in EEPROM we are going to be using floats and unions for bytes
//The data union contains 2 values, one for a float and one for 5 bytes
//the first 4 bytes within the data union will contain the bytes for each individual float, while the 5th corresponds to the
void handleSensorRequests(sensor_type sensor, def_message_struct *msg, int ind, char* cmd_passed) {
    int i;
    int valIndex = 0;
    EEPROMData data[4];
    switch (sensor) {
        case TEMP_AND_HUMID:
            if (sens_unit_ptr->dht_sensor != nullptr && ind == 0) {
                msg->values[0] = sens_unit_ptr->dht_sensor->readTemperature();
            } else if (sens_unit_ptr->dht_sensor != nullptr && ind == 1) {
                msg->values[0] = sens_unit_ptr->dht_sensor->readHumidity();
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
            } else {
                msg->message[0] = '\0';
                msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", "UNABLE TO FIND READING");
                msg->numValues = 0;
            }
            break;
        default:
            if (ind == 0) {
                determineStatus(msg);
            } else if (ind == 1) {
                returnSensUnits(msg);
            } else if (ind == 2) {
                msg->message[0] = '\0';
                if (sens_unit_ptr->name[0] != '\0') {
                    msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", sens_unit_ptr->name);
                } else {
                    msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", "NAME WAS NULL PUSH A NAME TO THIS DEVICE");
                }
            } else if (ind == 3) {
                sens_unit_ptr->name[0] = '\0';
                int len = strlen(cmd_passed);
                int i = 0;
                while (cmd_passed[i] != '|') {
                    i++;
                }
                if (i < len) {
                    substring(cmd_passed, i+1, (len-1-i+1), sens_unit_ptr->name, MAX_NAME_LEN);
                    msg->message[0] = '\0';
                    msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", sens_unit_response[3]);
                    strncat(msg->message, sens_unit_ptr->name, MAX_MSG_LENGTH-strlen(sens_unit_response[3]));
                    writeToEEPROM(sens_unit_ptr->name, msg);
                } else {
                    msg->message[0] = '\0';
                    msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", "Name failed to set due to invalid command being sent");
                }
            } else {
                msg->message[0] = '\0';
                msg->strlen = snprintf(msg->message, MAX_MSG_LENGTH, "%s", "INVALID INDEX PASSED");
            }
            break;        
    }
}


//Takes in the command passed and a default message struct to respond to
void handleRequestSU(char* cmd_passed, def_message_struct *response) {
    memset(response, 0, sizeof(def_message_struct));
    response->message[0] = '\0';
    if (sens_unit_ptr == nullptr) {
        response->strlen = snprintf(response->message, MAX_MSG_LENGTH, "%s", "sens_unit_ptr was never initialized");
        return;
    }
    int i;
    int j = 0;
    
    bool completed = false;
    for (i = 0; i < NUM_OF_SENSORS; i++) {
        while (sensors[i].commands[j]!=NULL) {
            if (strncmp(sensors[i].commands[j], cmd_passed, MAX_CMD_LENGTH) == 0) {
                response->strlen = snprintf(response->message, MAX_MSG_LENGTH, "%s", sensors[i].responses[j]);
                handleSensorRequests(sensors[i].sensor, response, j, cmd_passed);
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
    sendMessage(sens_unit_ptr->CU_ADDR, (uint8_t*)response, sizeof(*response));
}