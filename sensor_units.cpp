#include "sensor_units.h"
#define TEMP_SENSOR_IND 0
#define GPS_SENSOR_IND 1
#define TIME_SENSOR_IND 2


char* temp_sensor_cmds[] = {"PULL TEMP", "PULL HUMID", NULL};
char* temp_sensor_responses[] = {"TEMP", "HUMIDITY", NULL};

char* gps_sensor_cmds[] = {"PULL LOCATION", "PULL TIME", NULL};
char* gps_sensor_responses[] = {"Lat and long: ", "TIME", NULL};

char* time_sensor_cmds[] = {"PULL TIME", NULL};
char* time_sensor_cmds[] = {"Time", NULL};
char** allCmds[] = {temp_sensor_cmds, gps_sensor_cmds, time_sensor_cmds};



int handleSURequest(char* cmd_passed, sensor_unit *SU, def_message_struct *response) {
    sensor_type module;
    int i = 0;
    int j = 0;
    while (i < 3) {
        while (*(allCmds)[j] != NULL) {
            if (strncmp(cmd_passed, *(allCmds)[j], MAX_CMD_LENGTH) == 0) {
                module = (enum sensor_type)i;
                break;
            }
            j++;
        }
        j=0;
        i++;
    }
    if (module == TEMP_AND_HUMID) {
        handleTempRequests(cmd_passed, SU, response);
    } else if (module == GPS) {
        handleGpsRequests(cmd_passed, response, SU);
    } else if (module == TIME) {

    } else {
        return -1;
    }

}


int handleTempRequests(char* cmd_passed, sensor_unit *SU, def_message_struct *response) {
    if (strncmp(cmd_passed, temp_sensor_cmds[0], 16) == 0) {
        strncpy(response->message, temp_sensor_responses[0], 32);
        response->values[0] = SU->dht_sensor->readTemperature();
    } else if (strncmp(cmd_passed, temp_sensor_cmds[1], MAX_CMD_LENGTH)) {
        strncpy(response->message, temp_sensor_responses[1], 32);
        response->values[0] = SU->dht_sensor->readHumidity();
    } else {
        return -1;
    }
    return 0;
}


//GPS sensors are also equipped with fully capable time modules
int handleGpsRequests(char* cmd_passed, def_message_struct *response, sensor_unit *SU) {
    if (!strncmp(cmd_passed, gps_sensor_cmds[0], MAX_CMD_LENGTH) == 0 &&  SU->gpsSerial->available()) {
        strncpy(response->message, gps_sensor_responses[0], MAX_MSG_LENGTH);
        response->values[0] = SU->gps->location.lat();
        response->values[1] = SU->gps->location.lng();

    } else if (!strncmp(cmd_passed, "PULL TIME", MAX_CMD_LENGTH) && SU->gpsSerial->available()) {
        strncpy(response->message, gps_sensor_responses[1], MAX_MSG_LENGTH);
        
    } else if (!SU->gpsSerial->available()) {
        strncpy(response->message, "ERROR WITH GPS_SERIAL", MAX_CMD_LENGTH);
        return -1;
    }

    strncat(response->message, SU->SU_NAME, MAX_MSG_LENGTH);
    return 0;
}
