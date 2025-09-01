#include "sensor_units.h"

const char *temp_sensor_cmds[] = {"PULL TEMPERATURE", "PULL HUMID", NULL};
const char* temp_sensor_responses[] = {"TEMPERATURE", "HUMIDITY", NULL};

const char* gps_sensor_cmds[] = {"PULL LOCATION",NULL};
const char* gps_sensor_responses[] = {"LOCATION", NULL};

const char* sens_unit_msgs[] = {"PULL STATUS", "PULL SENS UNITS", "PULL NAME", "PUSH NAME", NULL};
const char* sens_unit_response[] = {"Status", "Sens units", "Name", "Name set", NULL};

const char* sens_unit_strings[] = {"Temperature and humidity", "GPS", NULL};
const char* status_strings[] = {"Online", "Error", "Offline",NULL};
//They need to mirror sens_unit_strings values
sensor_definition sensors[NUM_OF_SENSORS+1] = {
    {temp_sensor_cmds, temp_sensor_responses, TEMP_AND_HUMID, sens_unit_strings[0]},
    {gps_sensor_cmds, gps_sensor_responses, GPS, sens_unit_strings[1]},
    {sens_unit_msgs, sens_unit_response, BASE_SENS_UNIT, "DEFAULT"}
};

#ifdef LCD_I2C_ADDR
LCD_I2C LCD(LCD_I2C_ADDR, 16, 2);
#endif

