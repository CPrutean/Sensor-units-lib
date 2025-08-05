#include "sensor_units.h"

char* temp_sensor_cmds[] = {"PULL TEMP", "PULL HUMID", NULL};
char* temp_sensor_responses[] = {"TEMP", "HUMIDITY", NULL};

char* gps_sensor_cmds[] = {"PULL LOCATION", NULL};
char* gps_sensor_responses[] = {"Lat and long: ", NULL};

char* time_sensor_cmds[] = {"PULL TIME", NULL};
char* time_sensor_cmds[] = {"Time", NULL};

char** allCmds[] = {temp_sensor_cmds, gps_sensor_cmds, time_sensor_cmds, NULL};



int handleSURequest(char* cmd_passed, def_message_struct *response) {
    if (sens_unit_ptr == nullptr) {
        strncpy(response->message, "sens_unit_ptr wasnt initialized", MAX_MSG_LENGTH);
        return -1;
    }
    int i;
    int j = 0;
    EEPROMData data;
    
    for (i = 0; i < (int)NUM_OF_SENSORS; i++) {
        while (*(allCmds+i)[j] != NULL) {
            if (!strncmp(cmd_passed, *(allCmds+i)[j], MAX_CMD_LENGTH) && readFromEEPROM(sensor_type(i), j, &data)) {
                memset(&response, 0, sizeof(response));
                strncpy(response->message, *(allCmds+i)[j], MAX_MSG_LENGTH);
                response->values[0] = data.val;
                sendMessage(sens_unit_ptr->CU_ADDR, (uint8_t*)response, sizeof(*response));
                return 0;
            }
            j++;
        }
        j = 0;
    }
    return -1;
}


//For the sake of storing something in EEPROM we are going to be using floats and unions for bytes
//The data union contains 2 values, one for a float and one for 5 bytes
//the first 4 bytes within the data union will contain the bytes for each individual float, while the 5th corresponds to the
//index of the value sensors command index (EX: if we are pulling temp the value passed to the byte is 0 corresponding to index 0 of temp_sensor_cmds)
void readAll(sensor_unit *SU) {
    float readings[MAX_READINGS];
    enum sensor_type sensorHash[MAX_READINGS];
    int readingsInd;
    readTempAndHumid(readings, sensorHash, SU, &readingsInd);

    readGPSLatAndLong(readings, sensorHash, SU, &readingsInd);

    writeToEEPROM(readings, sensorHash, readingsInd+1);
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
    if (SU->gpsSerial != nullptr) {
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


void writeToEEPROM(float readings[MAX_READINGS], sensor_type sensorHash[MAX_READINGS], int numReadings) {
    clearEEPROM();

    if (EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("Succesfully initialized EEPROM");
    } else {
        Serial.println("Failed to intialize EEPROM");
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
    
    if (EEPROM.commit()) {
        Serial.print("EEPROM commit was succesful");
    } else {
        Serial.print("EEPROM commit was a failure");
    }
    EEPROM.end();
}



bool readFromEEPROM(sensor_type sensor, int ind, EEPROMData *data) {
    if (EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("Succesfully initialized EEPROM");
    } else {
        Serial.println("Failed to intialize EEPROM");
    }
    EEPROMData data;
    EEPROMData temp;
    int i;
    bool found = false;
    for (i = 0; i < EEPROM_SIZE; i+=sizeof(EEPROMData)) {
        EEPROM.get(i, temp);
        if ((sensor_type)temp.sensor == sensor && temp.ind == ind) {
            memcpy(data, &temp, sizeof(temp));
            found = true;
            break;
        }
    }
    EEPROM.end();
    return found;
}
