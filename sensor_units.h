#ifndef __SENSOR
#define __SENSOR
#include <Arduino.h>
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

typedef struct _sensor_unit {
    enum sensor_type modules[3];
    char available_commands[6][16];
    DHT *dht_sensor;
    HardwareSerial *gpsSerial;
    TinyGPSPlus *gps;
    _sensor_unit(sensor_type sensors[3], uint8_t DHT_SETUP[2]) {
        int i;
        int sensor_ind = 0;
        for (i = 0; i < 3; i++) {
            if (modules[i] == NULL) {
                modules[i] = sensors[sensor_ind++];
            }
        }
        dht_sensor->begin();
        gpsSerial->begin(GPS_BAUD);

    }
} sensor_unit;

typedef struct def_message_struct {
    char message[MAX_MSG_LENGTH];
    int urgency;
    float value;
    unsigned int MSG_ID;
} def_message_struct;

int return_available_commands(char** array, int len, enum sensor_type sensor);
int handleRequest(char* cmd_passed, def_message_struct *response, sensor_unit CU);

#endif