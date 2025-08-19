#pragma once

#include <Arduino.h>
#include <string.h>
#include <DHT.h>
#include <time.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <EEPROM.h>
#include <LCD_I2C.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#define GPS_BAUD 9600
#define MAX_MSG_LENGTH 100
#define MAX_CMD_LENGTH 32
#define DEBUG 1

#define EEPROM_SIZE 512
#define MAX_READINGS 80
#define MAX_NAME_LEN 30

#define MAX_QUEUE_LEN 25
#define MAX_ESP32_PEERS 10
//Needs to define PI_SERIAL in .ino files to implement communication by uart

extern char* temp_sensor_cmds[];
extern char* temp_sensor_responses[];

extern char* gps_sensor_cmds[];
extern char* gps_sensor_responses[];

extern char* sens_unit_msgs[];
extern char* sens_unit_response[];

extern char* sens_unit_strings[];
extern char* status_strings[];

enum sensor_type {TEMP_AND_HUMID = 0, GPS, NUM_OF_SENSORS};
enum sensor_unit_status {ONLINE = 0, ERROR, OFFLINE};

typedef struct def_message_struct {
    char message[MAX_MSG_LENGTH];
    uint8_t strlen;
    float values[4];
    uint8_t urgency;
    uint8_t numValues;
    uint8_t suInd;
} def_message_struct;

class msg_queue {
    public:
        bool send(const def_message_struct& msg);
        bool receive(def_message_struct& msg);
        QueueHandle_t getQueueHandle() const;
        msg_queue();
    private:
        QueueHandle_t queueHandle;
};


typedef struct EEPROMData {
    float val;
    uint8_t sensor;
    uint8_t ind;
} EEPROMData;


typedef struct sensor_definition {
    char** commands;
    char** responses;
    sensor_type sensor;
};

typedef struct sensor_unit{
    sensor_type* modules;
    uint8_t moduleCount;
    DHT *dht_sensor;
    HardwareSerial *gpsSerial;
    TinyGPSPlus *gps;
    uint8_t CU_ADDR[6];
    msg_queue *queue;
    char name[MAX_NAME_LEN];
};

typedef struct communication_unit{
    uint8_t SU_ADDR[6][6];
    sensor_unit_status status[6];
    sensor_type *SU_AVLBL_MODULES[6];
    uint8_t SU_NUM_MODULES[6];
    char** names;
    uint8_t numOfSU;
    msg_queue *queue;
};
 

extern sensor_definition sensors[];

//UN COMMENT THIS WHEN INITIALIZING THE LCD_I2C OBJECT
//Change the address as needed default i2c addresses for backpacks are 0x27
//#define LCD_I2C_ADDR 0x27

//Initialize these pointers within your .ino file and set them to the address of these individual objects
extern sensor_unit *sens_unit_ptr;
extern communication_unit *com_unit_ptr;

//Define LCD_I2C_ADDR for error handling and testing and visible erorr handling
#ifdef LCD_I2C_ADDR
extern LCD_I2C LCD;
#endif

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



void initCU(communication_unit *CU);

bool initSU(sensor_unit *SU);
bool readFromEEPROM(char* nameDest, int destSize, def_message_struct *msg);

int substring(const char* source, int start, int len, char* dest, int bufferLen);

int handleMSG_CU(const def_message_struct &msgRecv, int SUInd);
void handleRequestSU(char* cmd_passed, def_message_struct *response);

void readAll(sensor_unit *SU);
void clearEEPROM(); 
int sendMessage(uint8_t brdcstAddr[6], uint8_t* msg, int len);
void respondPiRequest(const char* str); 