#pragma once

#include <arduino.h>
#include <string.h>
#include <DHT.h>
#include <time.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <esp_now.h>
#include <WiFi.h>

/* TODO implement communication unit data protocols
implement error handling and fallbacks for faulty method calls and faulty information handling
implement communication unity data storage protocols and command handling
raspberry pi-esp32 communication protocols
*/
#define GPS_BAUD 9600
#define MAX_MSG_LENGTH 48
#define MAX_CMD_LENGTH 16  
#define MAX_QUEUE_LEN 20

//Define these with the sensor units name or the communication units name to allow preprocessor
//directive to determine which handlerequests method to compile
//DO NOT initialize both since this will cause a compiler error
#define SENSORUNIT
#define COMMUNICATIONUNIT

enum sensor_type {TEMP_AND_HUMID = 0, GPS = 1, TIME = 2};

//Initialize these pointers within your .ino file and set them to the address of these individual objects
sensor_unit *sens_unit_ptr;
communication_unit *com_unit_ptr;


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
//Pointers to validly initialized objects if 
typedef struct _sensor_unit {
    enum sensor_type modules[3];
    char* SU_NAME;
    DHT *dht_sensor;
    HardwareSerial *gpsSerial;
    TinyGPSPlus *gps;
    uint8_t CU_ADDR[6];
    esp_now_peer_info_t CU_PEER_INF;
    def_message_struct lastMsgSent;
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

/*
-Communication units will be able to communicate with sensor units and send them messages as well as serve as a web endpoint for other
devices and communicate with web devices

-Communication devices will be able to pull data from sensor units and communicate the information to the requesting endpoint


*/
typedef struct _communication_unit {
    char* commands[MAX_CMD_LENGTH]; 
    uint8_t SU_ADDR[6][6];
    char* SSID;
    char* PSWD;
    sensor_unit available_SU[6];
    esp_now_peer_info_t SU_PEER_INF[6];
    def_message_struct lastMsgSent;
} communication_unit;
 


typedef struct def_message_struct {
    char message[MAX_MSG_LENGTH];
    float values[2];
} def_message_struct;

//Pass in the amount of sensor units you are implementing as well as wifi network your joining and the SSID
int init_CU(sensor_unit *SU_arr, int len, communication_unit *CU, char* ssid, char* pswd);

//Intitialize the sensor unit with the values that you passed to it as well as the sensor attached to the unit itself
int init_SU_ESPNOW(sensor_unit *SU, int channel);


int handleRequestCU(def_message_struct msgRecv);
int handleRequestSU(char* cmd_passed, def_message_struct *response, sensor_unit *SU);

int sendMessage(uint8_t brdcstAddr[6], uint8_t* msg, int len);
void def_onDataRecv(const u_int8_t* adr, const u_int8_t* data, int len);
void def_onDataSent(const u_int8_t *addr, esp_now_send_status_t status);
