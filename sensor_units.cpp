#include "sensor_units.h"

char* temp_sensor_cmds[] = {"PULL TEMP", "PULL HUMID", NULL};
char* temp_sensor_responses[] = {"TEMP", "HUMIDITY", NULL};

char* gps_sensor_cmds[] = {"PULL LOCATION", NULL};
char* gps_sensor_responses[] = {"Lat and long: ", NULL};

char* time_sensor_cmds[] = {"PULL TIME", NULL};
char* time_sensor_cmds[] = {"Time", NULL};

char** allCmds[] = {temp_sensor_cmds, gps_sensor_cmds, time_sensor_cmds};



int handleSURequest(char* cmd_passed, def_message_struct *response) {
    if (sens_unit_ptr == nullptr) {
        strncpy(response->message, "sens_unit_ptr wasnt initialized", MAX_MSG_LENGTH);
        return -1;
    }
    int i;
    int j;
    union data val;
    for (i = 0; i < 3; i++) {
        while (*(allCmds+i)[j] != NULL) {
            if (strncmp(cmd_passed, *(allCmds+i)[j], MAX_CMD_LENGTH) == 0) {
                readFromEEPROM((enum sensor_type)i, j, &val);
                break;
            }
            j++;
        }
        j = 0;
    }
    memset(response, 0, sizeof(*response));
    strncpy(response->message, *(allCmds+i)[j], MAX_MSG_LENGTH);
    response->values[0] = val.val;
    return 0;
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
void readTempAndHumid(float* readings, enum sensor_type *sensorHash, sensor_unit *SU, int* readingsInd) {
    if (SU->dht_sensor != nullptr) {
        readings[*readingsInd] = SU->dht_sensor->readTemperature();
        sensorHash[*(readingsInd)++] = TEMP_AND_HUMID;
        
        readings[*readingsInd] = SU->dht_sensor->readHumidity();
        sensorHash[*(readingsInd)++] = TEMP_AND_HUMID;
    }
}

//GPS sensors are also equipped with fully capable time modules
void readGPSLatAndLong(float* readings, enum sensor_type *sensorHash, sensor_unit *SU, int* readingsInd) {
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
void writeToEEPROM(float readings[MAX_READINGS], enum sensor_type sensorHash[MAX_READINGS], int numReadings) {
    if (EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("Succesfully initialized EEPROM");
    } else {
        Serial.println("Failed to intialize EEPROM");
    }

    union data vals[numReadings];
    dataHash valsHash[numReadings];

    int i = 0;
    int hashInd = 0;
    int j;
    for (i = 0; i < numReadings; i++) {
        if (i > 0 && sensorHash[i] == sensorHash[i-1]) {
            hashInd++;
        } else {
            hashInd = 0;
        }
        vals[i].val = readings[i];
        vals[i].bytes[4] = (uint8_t)sensorHash[i];
        vals[i].bytes[5] = (uint8_t)hashInd;
        for (j = 0; j < 4; j++) {
            EEPROM.write((i*6)+j, vals[i].bytes[j]);
        }
    }

    if (EEPROM.commit()) {
        Serial.print("EEPROM commit was succesful");
    } else {
        Serial.print("EEPROM commit was a failure");
    }
    EEPROM.end();
}



void readFromEEPROM(sensor_type sensor, int ind, union data *val) {
    if (EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("Succesfully initialized EEPROM");
    } else {
        Serial.println("Failed to intialize EEPROM");
    }

    int i = 0;
    int j;
    while (i < EEPROM_SIZE) {
        if (EEPROM.read(i+4) == (uint8_t)sensor && EEPROM.read(i+5) == (uint8_t)ind) {
            for (j = 0; j < 4; j++) {
                val->bytes[j] = EEPROM.read(i+j);
            }
            val->bytes[4] = EEPROM.read(i+4);
            val->bytes[5] = EEPROM.read(i+5);
            return;
        }
        i+=6;
    }
    EEPROM.end();
}
