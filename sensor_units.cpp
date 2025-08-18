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
void writeToEEPROM(float readings[MAX_READINGS], sensor_type sensorHash[MAX_READINGS], int numReadings, def_message_struct *msg) {
    if (!EEPROM.begin(EEPROM_SIZE)) {
        msg->strlen += snprintf(msg->message, MAX_MSG_LENGTH, "%s", "FAILED TO INITIATE EEPROM");
    }
    int i;
    int hashInd = 0;
    EEPROMData arr[numReadings];

    int EEPROMInd = 0;
    int EEPROMSize = sizeof(EEPROMData);
    for (i = 0; i < numReadings; i++) {
        if (i > 0 && sensorHash[i] == sensorHash[i-1]) {
            hashInd++;
        } else {
            hashInd = 0;
        }
        arr[i].val = readings[i];
        arr[i].sensor = (uint8_t)sensorHash[i];
        arr[i].ind = hashInd;
        EEPROM.put(EEPROMInd, arr[i]);
        EEPROMInd += EEPROMSize;
    }
    
    if (!EEPROM.commit()) {
        msg->strlen += snprintf(msg->message, MAX_MSG_LENGTH, "%s", "FAILED TO COMMIT TO EEPROM");
    }
    EEPROM.end();
}


//Reads the EEPROM values stored
bool readFromEEPROM(sensor_type sensor, int ind, EEPROMData *data, def_message_struct *msg) {
    if (!EEPROM.begin(EEPROM_SIZE)) {
        msg->strlen += snprintf(msg->message, MAX_MSG_LENGTH, "%s", "FAILED TO INITIATE EEPROM");
    }
    EEPROMData temp;
    int i;
    bool found = false;
    for (i = 0; i < EEPROM_SIZE; i+=sizeof(EEPROMData)) {
        EEPROM.get(i, temp);
        if ((sensor_type)temp.sensor == sensor && temp.ind == ind) {
            memcpy(data, &temp, sizeof(temp));
            found = true;
            break;
        } else if (temp.ind == 0xff || temp.sensor == 0xff) {
            break;
        }
    }
    EEPROM.end();
    return found;
}


//Reads values from dht to stash in EEPROM flash memory
void readTempAndHumid(float* readings, sensor_type *sensorHash, sensor_unit *SU, int* readingsInd) {
    if (SU->dht_sensor != nullptr) {
        readings[*readingsInd] = SU->dht_sensor->readTemperature();
        sensorHash[*(readingsInd)++] = TEMP_AND_HUMID;
        
        readings[*readingsInd] = SU->dht_sensor->readHumidity();
        sensorHash[*(readingsInd)++] = TEMP_AND_HUMID;
    }
}

//GPS sensors are also equipped with fully capable time modules
//Reads values to store into EEPROM
void readGPSLatAndLong(float* readings, sensor_type *sensorHash, sensor_unit *SU, int* readingsInd) {
    if (SU->gpsSerial != nullptr && SU->gpsSerial->available()) {
        readings[*readingsInd] = SU->gps->location.lat();
        readings[*(readingsInd)++] = GPS;

        readings[*readingsInd] = SU->gps->location.lng();
        readings[*(readingsInd)++] = GPS;
    }
}

//Returns all the available sens units initialized in .ino files
void returnSensUnits(def_message_struct *msg) {
    strncpy(msg->message, sens_unit_response[1], MAX_MSG_LENGTH);
    int i = 0; 
    for (i = 0; i < sens_unit_ptr->moduleCount; i++) {
        msg->values[i] = (float)sens_unit_ptr->modules[i];
        msg->numValues++;
    }
}

//Collects data to write into EEPROM
void readAll(sensor_unit *SU, def_message_struct *msg) {
    float readings[MAX_READINGS];
    sensor_type sensorHash[MAX_READINGS];
    int readingsInd;
    readTempAndHumid(readings, sensorHash, SU, &readingsInd);

    readGPSLatAndLong(readings, sensorHash, SU, &readingsInd);

    writeToEEPROM(readings, sensorHash, readingsInd+1, msg);
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


//For the sake of storing something in EEPROM we are going to be using floats and unions for bytes
//The data union contains 2 values, one for a float and one for 5 bytes
//the first 4 bytes within the data union will contain the bytes for each individual float, while the 5th corresponds to the
void handleSensorRequests(sensor_type sensor, def_message_struct *msg, int ind) {
    int i;
    int valIndex = 0;
    EEPROMData data[4];
    switch (sensor) {
        case TEMP_AND_HUMID:
            if (sens_unit_ptr->dht_sensor != nullptr && ind == 0) {
                msg->values[0] = sens_unit_ptr->dht_sensor->readTemperature();
            } else if (sens_unit_ptr->dht_sensor != nullptr && ind == 1) {
                msg->values[0] = sens_unit_ptr->dht_sensor->readHumidity();
            } else if (sens_unit_ptr->dht_sensor != nullptr && readFromEEPROM(sensor, ind, &data[0], msg)) {
                msg->values[0] = data[0].val;
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
            } else if (ind == 0 && readFromEEPROM(sensor, ind, &data[0], msg) && readFromEEPROM(sensor, ind, &data[1], msg)) {
                msg->values[0] = data[0].val;
                msg->values[1] = data[1].val;
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
    if (sens_unit_ptr == nullptr) {
        strncpy(response->message, "sens_unit_ptr wasnt initialized", MAX_MSG_LENGTH);
        response->message[strlen(response->message)] = '\0';
        return;
    }
    int i;
    int j = 0;
    
    memset(response, 0, sizeof(*response));
    response->message[0] = '\0';
    bool completed = false;
    for (i = 0; i < NUM_OF_SENSORS; i++) {
        while (sensors[i].commands[j]!=NULL) {
            if (strncmp(sensors[i].commands[j], cmd_passed, MAX_CMD_LENGTH)) {
                response->strlen = snprintf(response->message, MAX_MSG_LENGTH, "%s", sensors[i].commands[j]);
                handleSensorRequests(sensors[i].sensor, response, j);
                completed = true;
                break;
            }
        }
        if (completed) {
            break;
        }
    }
    response->channel = sens_unit_ptr->CU_PEER_INF.channel;
    if (!completed) {
        memset(response, 0, sizeof(response));
        response->strlen = snprintf(response->message, MAX_MSG_LENGTH, "%s", "FAILED TO FIND REQUESTED VALUES");
    }
    sendMessage(sens_unit_ptr->CU_ADDR, (uint8_t*)response, sizeof(*response));
}







