#include "sensor_units.h"

const char *temp_sensor_cmds[] = {"PULL TEMPERATURE", "PULL HUMID", NULL};
const char* temp_sensor_responses[] = {"TEMPERATURE", "HUMIDITY", NULL};
const char* temp_sensor_values[] = {"C\xC2", "%", NULL};

const char* gps_sensor_cmds[] = {"PULL LOCATION",NULL};
const char* gps_sensor_responses[] = {"LOCATION", NULL};
const char* gps_sensor_values[] = {"\xC2", NULL};

const char* sens_unit_msgs[] = {"PULL STATUS", "PULL SENS UNITS", "PULL NAME", "PUSH NAME", NULL};
const char* sens_unit_response[] = {"Status", "Sens units", "Name", "Name set", NULL};
const char* sens_unit_values[] = {"STATUS CODE", "SENSOR UNITS", "NAME", NULL};

const char* motion_unit_msgs[] = {"PULL MOTION", NULL};
const char* motion_unit_response[] = {"MOTION DETECTED", NULL};
const char* motion_unit_values[] = {"Movement", NULL};


const char* sens_unit_strings[] = {"Temperature and humidity", "GPS", "Motion sensor", NULL};
const char* status_strings[] = {"Online", "Error", "Offline",NULL};

//They need to mirror sens_unit_strings values
sensor_definition sensors[NUM_OF_SENSORS+1] = {
    {temp_sensor_cmds, temp_sensor_responses, temp_sensor_values, TEMP_AND_HUMID, sens_unit_strings[0]},
    {gps_sensor_cmds, gps_sensor_responses, gps_sensor_values, GPS, sens_unit_strings[1]},
    {motion_unit_msgs, motion_unit_response, motion_unit_values, },
    {sens_unit_msgs, sens_unit_response, sens_unit_values, BASE_SENS_UNIT, "DEFAULT"}
};

#ifdef LCD_I2C_ADDR
LCD_I2C LCD(LCD_I2C_ADDR, 16, 2);
#endif