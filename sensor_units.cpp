#include "sensor_units.h"

const char* temp_sensor_cmds[2] = {"PULL TEMP", "PULL HUMID"};
const char* temp_sensor_responses[2] = {"TEMP", "HUMIDITY"};

const char* gps_sensor_cmds[2] = {"PULL LOCATION", "PULL TIME"};
const char* gps_sensor_responses[2] = {"Lat and long: ", "TIME"};

const char* time_sensor_cmds[1] = {"PULL TIME"};
const char* time_sensor_cmds[1] = {"Time"};


//Pass in an array with unlimited rows and the columns being 16 long
int return_available_commands(char** array, int len, enum sensor_type sensor) {
    if (len != MAX_CMD_LENGTH) {
        return -1;
    }
    int i;
    int len;
    if (sensor == TEMP_AND_HUMID) {
        len = sizeof(temp_sensor_cmds)/sizeof(temp_sensor_cmds[0]);
        for (i = 0; i < len; i++) {
            strncpy(*(array+i), temp_sensor_cmds[i], MAX_CMD_LENGTH);
        }
    } else if (sensor == GPS) {
        len = sizeof(gps_sensor_cmds)/sizeof(gps_sensor_cmds[0]);
        for (i = 0; i < len; i++) {
            strncpy(*(array+i), gps_sensor_cmds[i], MAX_CMD_LENGTH);
        }
    } else {
        return -1;
    }
    return 0;
}

int handleRequest(enum sensor_type module, char* cmd_passed, def_message_struct *response, sensor_unit SU) {
    memset(response, 0, sizeof(response));
    if (module == TEMP_AND_HUMID) {
        handleTempRequests(cmd_passed, response, SU);
    } else if (module = GPS) {
        handleGpsRequests(cmd_passed, response, SU);
    } else if (module = TIME) {
        
    } else {
        return -1;
    }
    return 0;
}

int handleTempRequests(char* cmd_passed, def_message_struct *response, sensor_unit SU) {
    if (strncmp(cmd_passed, temp_sensor_cmds[0], 16) == 0) {
        strncpy(response->message, temp_sensor_responses[0], 32);
        response->value = SU.dht_sensor->readTemperature();
    } else if (strncmp(cmd_passed, temp_sensor_cmds[1], MAX_CMD_LENGTH)) {
        strncpy(response->message, temp_sensor_responses[1], 32);
        response->value = SU.dht_sensor->readHumidity();
    } else {
        return -1;
    }
    return 0;
}


//GPS sensors are also equipped with fully capable time modules
int handleGpsRequests(char* cmd_passed, def_message_struct *response, sensor_unit SU) {
    if (!strncmp(cmd_passed, "PULL TEMP", MAX_CMD_LENGTH) == 0) {
        strncpy(response->message, gps_sensor_responses[0], MAX_MSG_LENGTH);
    } else if (!strncmp(cmd_passed, "PULL TIME", MAX_CMD_LENGTH)) {
        strncpy(response->message, gps_sensor_responses[1], MAX_MSG_LENGTH);
    } else {
        return -1;
    }
    return 0;
}



int main() {
    return 0;
}