#pragma once

#include <arduino.h>
#include <string.h>
#include <DHT.h>
#include <time.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <esp_now.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <LCD_I2C.h>
#include <mutex>
#include <stdexcept>
#include <algorithm>
#include <vector>

#define GPS_BAUD 9600
#define MAX_MSG_LENGTH 48
#define MAX_CMD_LENGTH 16  

#define EEPROM_SIZE 512
#define MAX_READINGS 80

#define MAX_QUEUE_LEN 25

#define NULL_VALUE 0xff


const char* temp_sensor_cmds[] = {"PULL TEMP", "PULL HUMID", NULL};
const char* temp_sensor_responses[] = {"TEMP", "HUMIDITY", NULL};

const char* gps_sensor_cmds[] = {"PULL LOCATION",NULL};
const char* gps_sensor_responses[] = {"LOCATION", NULL};

const char* sens_unit_msgs[] = {"GET STATUS", "RETURN SENS UNITS", "", NULL};
const char* sens_unit_response[] = {"Status", "Sens_units"};

const char* sens_unit_strings[] = {"Temp and humidity", "GPS", NULL};
const char* status_strings[] = {"Online", "Error", "Offline",NULL};

enum sensor_type {TEMP_AND_HUMID = 0, GPS, NUM_OF_SENSORS};
enum sensor_unit_status {ONLINE = 0, ERROR, OFFLINE};

typedef struct sensor_definition {
    const char** commands;
    const char** responses;
    sensor_type sensor;
} sensor_definition;

sensor_definition sensors[NUM_OF_SENSORS+1] = {
    {temp_sensor_cmds, temp_sensor_responses, TEMP_AND_HUMID},
    {gps_sensor_cmds, gps_sensor_responses, GPS},
    {sens_unit_msgs, sens_unit_response, NUM_OF_SENSORS}
};

//Pointers to validly initialized objects if 
typedef struct _sensor_unit {
    sensor_type modules[3];
    DHT *dht_sensor;
    HardwareSerial *gpsSerial;
    TinyGPSPlus *gps;
    uint8_t CU_ADDR[6];
    esp_now_peer_info_t CU_PEER_INF;
    msg_queue *queue;
} sensor_unit;

typedef struct _communication_unit {
    uint8_t SU_ADDR[6][6];
    esp_now_peer_info_t SU_PEER_INF[6];
    sensor_unit_status status[6];
    sensor_type SU_AVLBL_MODULES[6][3];
    msg_queue *queue;
} communication_unit;
 


typedef struct def_message_struct {
    char message[MAX_MSG_LENGTH];
    float values[4];
    uint8_t channel;
    uint8_t urgency;
} def_message_struct;


//For EEPROM readings we will take a float pass it into EEPROM read and write cycles.
//The 5th byte represents the index of the command needed to return the temperature and the humidity
//Example: (If the value of the 5th byte is 0 that means were requesting the temperature as that corresponds to index 0 of the array within the temperature commands)
typedef struct EEPROMData {
    float val;
    uint8_t sensor;
    uint8_t ind;
} EEPROMData;

class msg_queue {
    public:
        msg_queue();
        bool add(const def_message_struct msg);
        bool clear();
        bool pop();
        def_message_struct getFront() const;
        size_t getSize() const;
        bool isEmpty() const;
    private:
        std::vector<def_message_struct> msgs;
        mutable std::mutex q_mutex;
};

//UN COMMENT THIS WHEN INITIALIZING THE LCD_I2C OBJECT
//Change the address as needed default i2c addresses for backpacks are 0x27
//#define LCD_I2C_ADDR 0x27

//Initialize these pointers within your .ino file and set them to the address of these individual objects
sensor_unit *sens_unit_ptr;
communication_unit *com_unit_ptr;

//Define LCD_I2C_ADDR for error handling and testing and visible erorr handling
#ifdef LCD_I2C_ADDR
LCD_I2C LCD(LCD_I2C_ADDR, 16, 2);
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



//Pass in the amount of sensor units you are implementing as well as wifi network your joining and the SSID
int init_CU(communication_unit *CU);

//Intitialize the sensor unit with the values that you passed to it as well as the sensor attached to the unit itself
int init_SU_ESPNOW(sensor_unit *SU, int channel);


int handleMSG_CU(def_message_struct msgRecv, int channel);
int handleRequestSU(char* cmd_passed, def_message_struct *response);
void readAll(sensor_unit *SU);
int sendMessage(uint8_t brdcstAddr[6], uint8_t* msg, int len);
void def_onDataRecv(const u_int8_t* adr, const u_int8_t* data, int len);
void def_onDataSent(const u_int8_t *addr, esp_now_send_status_t status);
