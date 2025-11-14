#include "sensor_units.h"

// Each sensor method will have the structure of returnType
// methodHeader(def_message_struct *msgRecv) functionBody
//
// that way the structure for all method calls is simple and has the same
// behavior however the method calls will be declared at runtime due to
// sens_unit_ptr being a runtime declared variable
const char *temp_sensor_cmds[] = {"PULL TEMPERATURE", "PULL HUMID", "PULL PREF", "PUSH PREF", NULL};
const char *temp_sensor_responses[] = {"TEMPERATURE", "HUMIDITY", "PREF", "PREF SET", NULL};
const def_message_struct_DATA_TYPES temp_sensor_values[] = {DOUBLE_T, DOUBLE_T, STRING_T, STRING_T}; 


const char *gps_sensor_cmds[] = {"PULL LOCATION", NULL};
const char *gps_sensor_responses[] = {"LOCATION", NULL};
const def_message_struct_DATA_TYPES gps_sensor_values[] = {DOUBLE_T}; 


const char *motion_unit_msgs[] = {"PULL MOTION", NULL};
const char *motion_unit_response[] = {"MOTION DETECTED", NULL};
const def_message_struct_DATA_TYPES motion_unit_values[] = {DOUBLE_T}; 


const char *sens_unit_msgs[] = {"PULL STATUS", "PULL SENS UNITS", "PULL NAME", "PUSH NAME", NULL};
const char *sens_unit_response[] = {"Status", "Sens units", "Name", "Name set", NULL};
const def_message_struct_DATA_TYPES sens_unit_values[] = {STRING_T, STRING_T, STRING_T, STRING_T}; 


const char *sens_unit_strings[] = {"Temperature and humidity", "GPS",
                                   "Motion sensor", NULL};
const char *status_strings[] = {"Online", "Error", "Offline", NULL};



// They need to mirror sens_unit_strings values
sensor_definition sensors[NUM_OF_SENSORS + 1] = {
    {temp_sensor_cmds, temp_sensor_responses, temp_sensor_values,
     TEMP_AND_HUMID, sens_unit_strings[0]},
    {gps_sensor_cmds, gps_sensor_responses, gps_sensor_values, GPS,
     sens_unit_strings[1]},
    {motion_unit_msgs, motion_unit_response, motion_unit_values, MOTION_SENSOR,
     sens_unit_strings[MOTION_SENSOR]},
    {sens_unit_msgs, sens_unit_response, sens_unit_values, BASE_SENS_UNIT,
     "DEFAULT"}};

sensor_unit *sens_unit_ptr = nullptr;
communication_unit *com_unit_ptr = nullptr;

#ifdef LCD_I2C_ADDR
LCD_I2C LCD(LCD_I2C_ADDR, 16, 2);
#endif
