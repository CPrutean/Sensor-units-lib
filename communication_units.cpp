#include "sensor_units.h"

char* temp_sensor_cmds[] = {"PULL TEMP", "PULL HUMID", NULL};
char* temp_sensor_responses[] = {"TEMP", "HUMIDITY", NULL};

char* gps_sensor_cmds[] = {"PULL LOCATION",NULL};
char* gps_sensor_responses[] = {"LOCATION", NULL};

char* sens_unit_msgs[] = {"GET STATUS", "RETURN SENS UNITS", "", NULL};
char* sens_unit_response[] = {"Status", "Sens_units"};

char* sens_unit_strings[] = {"Temp and humidity", "GPS", NULL};
char* status_strings[] = {"Online", "Error", "Offline",NULL};

sensor_definition sensors[NUM_OF_SENSORS+1] = {
    {temp_sensor_cmds, temp_sensor_responses, TEMP_AND_HUMID},
    {gps_sensor_cmds, gps_sensor_responses, GPS},
    {sens_unit_msgs, sens_unit_response, NUM_OF_SENSORS}
};




int handleMSG_CU(def_message_struct msgRecv) {
    if (com_unit_ptr == nullptr) {
        return -1;
    }

    String returnVal{""};
    returnVal += msgRecv.message;
    int i;
    if (strncmp(msgRecv.message, sens_unit_response[0], MAX_CMD_LENGTH)) {
        
    } else if (strncmp(msgRecv.message, sens_unit_response[1], MAX_CMD_LENGTH)) {
        
    }
    returnVal += "|SU";
    returnVal +=  std::to_string(msgRecv.channel+1);

}

void stageForReturn(def_message_struct msg) {

}








