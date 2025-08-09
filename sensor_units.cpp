#include "sensor_units.h"


void handleSURequest(char* cmd_passed, def_message_struct *response) {
    memset(response, 0, sizeof(def_message_struct));
    if (sens_unit_ptr == nullptr) {
        strncpy(response->message, "sens_unit_ptr wasnt initialized", MAX_MSG_LENGTH);
        return;
    }
    int i;
    int j = 0;
    
    memset(response, 0, sizeof(*response));
    bool completed = false;
    for (i = 0; i < NUM_OF_SENSORS; i++) {
        while (sensors[i].commands[j]!=NULL) {
            if (strncmp(sensors[i].commands[j], cmd_passed, MAX_CMD_LENGTH)) {
                strncpy(response->message, sensors[i].responses[j], MAX_MSG_LENGTH);
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
    sendMessage(sens_unit_ptr->CU_ADDR, (uint8_t*)response, sizeof(*response));
}


//For the sake of storing something in EEPROM we are going to be using floats and unions for bytes
//The data union contains 2 values, one for a float and one for 5 bytes
//the first 4 bytes within the data union will contain the bytes for each individual float, while the 5th corresponds to the
//index of the value sensors command index (EX: if we are pulling temp the value passed to the byte is 0 corresponding to index 0 of temp_sensor_cmds)
void handleSensorRequests(sensor_type sensor, def_message_struct *msg, int ind) {
    int i;
    int valIndex = 0;
    EEPROMData data[4];
    switch (sensor) {
        case TEMP_AND_HUMID:
            if (readFromEEPROM(sensor, ind, &data[0], msg)) {
                msg->values[valIndex++] = data[0].val;
                msg->values[valIndex++] = data[1].val;
            } else {
                strncpy(msg->message, "ERR FAILED TO FIND READING", MAX_MSG_LENGTH);
            }
            break;
        case GPS:
            if (readFromEEPROM(sensor, 0, &data[0], msg) && readFromEEPROM(sensor, 1, &data[1], msg)) {
                msg->values[valIndex++] = data[0].val;
                msg->values[valIndex++] = data[1].val;
            } else {
                strncpy(msg->message, "ERR FAILED TO FIND READING OR READINGS", MAX_MSG_LENGTH);
            }
            break;
        default:
            if (ind == 0) {
                determineStatus(msg);
            } else if (ind == 1) {
                returnSensUnits(msg);
            }
            break;        
    }
    for (i = valIndex; i < 4; i++) {
        msg->values[i] = NULL_VALUE;
    }
}

void determineStatus(def_message_struct *msg) {
    if (sens_unit_ptr == nullptr) {
        strncpy(msg->message, sens_unit_response[0], MAX_MSG_LENGTH);
        strncat(msg->message, ": sens_unit_ptr was never initialized", MAX_MSG_LENGTH - strlen(msg->message) - 1);
        msg->values[0] = (float)ERROR;
        return;
    }

    msg->values[0] = (float)ONLINE; 
    strncpy(msg->message, sens_unit_response[0], MAX_MSG_LENGTH); 

    for (int i = 0; sens_unit_ptr->SU_AVLBL_MODULES[i] != NULL_VALUE; i++) { // Assuming NULL_SENSOR_TYPE terminates the array
        sensor_type current_sensor = sens_unit_ptr->SU_AVLBL_MODULES[i];
        
        if (current_sensor == TEMP_AND_HUMID && (sens_unit_ptr->dht_sensor == nullptr || isnan(sens_unit_ptr->dht_sensor.readTemperature()))) {
            msg->values[0] = (float)ERROR;
            strncat(msg->message, " DHT_FAIL", MAX_MSG_LENGTH - strlen(msg->message) - 1);
        } 
        else if (current_sensor == GPS && (sens_unit_ptr->gpsSerial == nullptr || sens_unit_ptr->gps == nullptr || !sens_unit_ptr->gps.available())) {
            msg->values[0] = (float)ERROR;
            strncat(msg->message, " GPS_FAIL", MAX_MSG_LENGTH - strlen(msg->message) - 1);
        }
    }
}

void returnSensUnits(def_message_struct *msg) {
    strncpy(msg->message, sens_unit_response[1], MAX_MSG_LENGTH);
    int i = 0; 
    while (sens_unit_ptr->SU_AVLBL_MODULES[i] != NULL) {
        msg->values[i] = (float)sens_unit_ptr->SU_AVLBL_MODULES[i];
    }
}

void readAll(sensor_unit *SU, def_message_struct *msg) {
    float readings[MAX_READINGS];
    sensor_type sensorHash[MAX_READINGS];
    int readingsInd;
    readTempAndHumid(readings, sensorHash, SU, &readingsInd);

    readGPSLatAndLong(readings, sensorHash, SU, &readingsInd);

    writeToEEPROM(readings, sensorHash, readingsInd+1, msg);
}

//
void readTempAndHumid(float* readings, sensor_type *sensorHash, sensor_unit *SU, int* readingsInd) {
    if (SU->dht_sensor != nullptr) {
        readings[*readingsInd] = SU->dht_sensor->readTemperature();
        sensorHash[*(readingsInd)++] = TEMP_AND_HUMID;
        
        readings[*readingsInd] = SU->dht_sensor->readHumidity();
        sensorHash[*(readingsInd)++] = TEMP_AND_HUMID;
    }
}

//GPS sensors are also equipped with fully capable time modules
void readGPSLatAndLong(float* readings, sensor_type *sensorHash, sensor_unit *SU, int* readingsInd) {
    if (SU->gpsSerial != nullptr && SU->gpsSerial->available()) {
        readings[*readingsInd] = SU->gps->location.lat();
        readings[*(readingsInd)++] = GPS;

        readings[*readingsInd] = SU->gps->location.lng();
        readings[*(readingsInd)++] = GPS;
    }
}

//Writes all values passed to EEPROM
//To prevent excessive read and write limits writings will only be staged to commit when youre pushing multiple commands
//Addresses will be stored after the available index for information

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


void writeToEEPROM(float readings[MAX_READINGS], sensor_type sensorHash[MAX_READINGS], int numReadings, def_message_struct *msg) {
    if (!EEPROM.begin(EEPROM_SIZE)) {
        strncat(msg->message, "ERROR FAILED TO INITIALIZE EEPROM", MAX_MSG_LENGTH);
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
        strncat(msg->message, "EEPROM COMMIT WAS A FAILURE ERROR", MAX_MSG_LENGTH);
    }
    EEPROM.end();
}



bool readFromEEPROM(sensor_type sensor, int ind, EEPROMData *data, def_message_struct *msg) {
    if (!EEPROM.begin(EEPROM_SIZE)) {
        strncat(msg->message, "ERROR FAILED TO INITIALIZE EEPROM", MAX_MSG_LENGTH);
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