#pragma once

#include <arduino.h>
#include <string.h>
#include <DHT.h>
#include <time.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <esp_now.h>
#include <WiFi.h>

//TODO Implement library response functions, CU espNOW initializations, SU communications with CU, espNOW acknowledgment protocols and scheduling systems using RTOS
#define GPS_BAUD 9600
#define MAX_MSG_LENGTH 48
#define MAX_CMD_LENGTH 16  
#define MAX_QUEUE_LEN 20
#define CU
#define SU

enum sensor_type {TEMP_AND_HUMID, GPS, TIME};

sensor_unit *sens_unit_ptr;
communication_unit *com_unit_ptr;
bool *newMsgPtr;
msg_queue *queue;


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
    sensor_type sensors_avlbl[3]; 
    uint8_t SU_ADDR[6][6];
    char* SSID;
    char* PSWD;
    sensor_unit available_SU[6];
    esp_now_peer_info_t SU_PEER_INF[6];
    def_message_struct lastMsgSent;
} communication_unit;
 


typedef struct def_message_struct {
    char message[MAX_MSG_LENGTH];
    int urgency;
    float values[2];
    unsigned short MSG_ID;
} def_message_struct;

typedef struct msg_queue {
    def_message_struct messages[MAX_QUEUE_LEN];
    int recievedAtMillis[MAX_QUEUE_LEN];
    int lastQueueInd;
    int* millis;
} msg_queue;

int init_CU(sensor_unit *SU_arr, int len, communication_unit *CU, char* ssid, char* pswd);

int handleSURequest(char* cmd_passed, def_message_struct *response, sensor_unit CU);
int handleCURequest(char* cmd_passed, def_message_struct *response, sensor_unit CU);

int sendMessage(uint8_t brdcstAddr[6], uint8_t* msg, int len);
void def_onDataRecv(const u_int8_t* adr, const u_int8_t* data, int len);
void def_onDataSent(const u_int8_t *addr, esp_now_send_status_t status);

//Will pop the first index in line
void pop(msg_queue *queue);
//Will pop the index given
void popAtInd(msg_queue *queue, int ind);
//Adds a message to the end of the queue
void addToQueue(def_message_struct, msg_queue *queue);
//Adds a message to the queue at an index
void addToQueueInd(msg_queue *queue, int ind);
