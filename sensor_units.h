#ifndef __SENSOR
#define __SENSOR
#include <arduino.h>
#include <string.h>
#include <DHT.h>
#include <time.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>


#define GPS_BAUD 9600
#define MAX_MSG_LENGTH 48
#define MAX_CMD_LENGTH 16

enum sensor_type {TEMP_AND_HUMID, GPS, TIME};

/*
-Certain things will be handled within the .ino files for the sensor units themselves

-Initializing the hardware serial objects will be handled within the sensor methods

-EspNOW communication will be handled within different sensor units .ino files however communication commands and responses will be handled 
using the library

-However standard message structure will be declared here as well as replies to pull and push
requests

-Sensor units will define themselves based off the structs and reply to communication units based on what
sensor types are declared within their localized struct

-Simpler sensors like the temperature sensors will be handled within the library
*/
//Pointers to validly initialized objects.
typedef struct _sensor_unit {
    enum sensor_type modules[3];
    DHT *dht_sensor;
    HardwareSerial *gpsSerial;
    TinyGPSPlus *gps;
    uint8_t CU_ADDR[6];
    _sensor_unit(sensor_type sensors[3]) {
        int i;
        int sensor_ind = 0;
        for (i = 0; i < 3; i++) {
            if (modules[i] == NULL) {
                modules[i] = sensors[sensor_ind++];
            }
        }
    }
} sensor_unit;

//Each communication unit will be able to communicate with 6 other sensor units
typedef struct _communication_unit {
    char* commands[16];
    char* SSID;
    char* PSWD;
    sensor_unit available_SU[6];
} communication_unit;
 


typedef struct def_message_struct {
    char message[MAX_MSG_LENGTH];
    int urgency;
    float values[4];
    unsigned int MSG_ID;
} def_message_struct;

int return_available_commands(char** array, int len, enum sensor_type sensor);
int handleRequest(char* cmd_passed, def_message_struct *response, sensor_unit CU);

#endif