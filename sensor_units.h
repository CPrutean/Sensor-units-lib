#pragma once
#include <Arduino.h>
#include <string.h>
#include <DHT.h>
#include <time.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <esp_now.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <LCD_I2C.h>
#include <cstdio>
#include <cstdlib>


#define GPS_BAUD 9600
#define MAX_MSG_LENGTH 100
#define MAX_CMD_LENGTH 32

#define EEPROM_SIZE 512
#define MAX_READINGS 80

#define MAX_QUEUE_LEN 25



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

#ifdef __cplusplus
extern"C" {
#endif
typedef struct def_message_struct {
    char message[MAX_MSG_LENGTH];
    uint8_t strlen;
    float values[4];
    uint8_t channel;
    uint8_t urgency;
    uint8_t numValues;
} def_message_struct;

typedef struct EEPROMData {
    float val;
    uint8_t sensor;
    uint8_t ind;
} EEPROMData;


typedef struct sensor_definition {
    char** commands;
    char** responses;
    sensor_type sensor;
} sensor_definition;

typedef struct sensor_unit{
    sensor_type modules[3];
    uint8_t moduleCount;
    DHT *dht_sensor;
    HardwareSerial *gpsSerial;
    TinyGPSPlus *gps;
    uint8_t CU_ADDR[6];
    esp_now_peer_info_t CU_PEER_INF;
    msg_queue *queue;
} sensor_unit;

typedef struct communication_unit{
    uint8_t SU_ADDR[6][6];
    esp_now_peer_info_t SU_PEER_INF[6];
    sensor_unit_status status[6];
    sensor_type SU_AVLBL_MODULES[6][3];
    msg_queue *queue;
} communication_unit;
 
#ifdef __cplusplus
}
#endif

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
        def_message_struct msgs[MAX_QUEUE_LEN];
        size_t sizeOfArray;
        SemaphoreHandle_t queue_mutex;
};

extern sensor_definition sensors[NUM_OF_SENSORS+1];

//UN COMMENT THIS WHEN INITIALIZING THE LCD_I2C OBJECT
//Change the address as needed default i2c addresses for backpacks are 0x27
//#define LCD_I2C_ADDR 0x27

//Initialize these pointers within your .ino file and set them to the address of these individual objects
extern sensor_unit *sens_unit_ptr;
extern communication_unit *com_unit_ptr;

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
int init_CU_ESPNOW(communication_unit *CU);

//Intitialize the sensor unit with the values that you passed to it as well as the sensor attached to the unit itself
int init_SU_ESPNOW(sensor_unit *SU, int channel);


int handleMSG_CU(def_message_struct msgRecv, int channel);
void handleRequestSU(char* cmd_passed, def_message_struct *response);

void readAll(sensor_unit *SU);
void clearEEPROM(); 
int sendMessage(uint8_t brdcstAddr[6], uint8_t* msg, int len);
void def_onDataRecv(const u_int8_t* adr, const u_int8_t* data, int len);
void def_onDataSent(const u_int8_t *addr, esp_now_send_status_t status);
void respondPiRequest(const char* str);
