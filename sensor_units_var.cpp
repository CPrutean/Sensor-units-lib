#include "sensor_units.h"

char *temp_sensor_cmds[] = {"PULL TEMP", "PULL HUMID", NULL};
char* temp_sensor_responses[] = {"TEMP", "HUMIDITY", NULL};

char* gps_sensor_cmds[] = {"PULL LOCATION",NULL};
char* gps_sensor_responses[] = {"LOCATION", NULL};

char* sens_unit_msgs[] = {"PULL STATUS", "PULL SENS UNITS", "", NULL};
char* sens_unit_response[] = {"Status", "Sens_units", NULL};

char* sens_unit_strings[] = {"Temp and humidity", "GPS", NULL};
char* status_strings[] = {"Online", "Error", "Offline",NULL};

sensor_definition sensors[NUM_OF_SENSORS+1] = {
    {temp_sensor_cmds, temp_sensor_responses, TEMP_AND_HUMID},
    {gps_sensor_cmds, gps_sensor_responses, GPS},
    {sens_unit_msgs, sens_unit_response, NUM_OF_SENSORS}
};

#ifdef LCD_I2C_ADDR
LCD_I2C LCD(LCD_I2C_ADDR, 16, 2);
#endif

