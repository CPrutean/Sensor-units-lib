#include "sensor_units.h"

const char* temp_sensor_cmds[2] = {"PULL TEMP", "PULL HUMID"};
const char* gps_sensor_cmds[2] = {"PULL LOCATION", "PULL TIME"};
const char* time_sensor_cmds[1] = {"PULL TIME"};

int handleRequestCU(def_message_struct msgRecv, const uint8_t *adr) {
    if (com_unit_ptr == nullptr) {
        return -1;
    }


    if (strncmp(msgRecv.message, "ONLINE", MAX_CMD_LENGTH) == 0) {
        
    }
}








