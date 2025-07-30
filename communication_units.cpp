#include "sensor_units.h"

const char* temp_sensor_cmds[2] = {"PULL TEMP", "PULL HUMID"};
const char* gps_sensor_cmds[2] = {"PULL LOCATION", "PULL TIME"};
const char* time_sensor_cmds[1] = {"PULL TIME"};

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
