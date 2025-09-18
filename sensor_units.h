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
//Define this when you want to better read serial monitor output
//#define DEBUG 1

#define EEPROM_SIZE 512
#define MAX_READINGS 80
#define MAX_NAME_LEN 30

#define MAX_QUEUE_LEN 25
#define MAX_ESP32_PEERS 10


//Default defined in sensor_units_var.cpp
extern const char* temp_sensor_cmds[];
extern const char* temp_sensor_responses[];

extern const char* gps_sensor_cmds[];
extern const char* gps_sensor_responses[];

extern const char* sens_unit_msgs[];
extern const char* sens_unit_response[];

extern const char* sens_unit_strings[];
extern const char* status_strings[];

enum sensor_type {TEMP_AND_HUMID = 0, GPS, BASE_SENS_UNIT, NUM_OF_SENSORS};
enum sensor_unit_status {ONLINE = 0, ERROR, OFFLINE};


//The default messages sent to and from the communication and sensor units
typedef struct def_message_struct {
    char message[MAX_MSG_LENGTH];
    uint8_t strlen;
    float values[4];
    uint8_t urgency;
    uint8_t numValues;
    uint8_t senderMac[6];
    unsigned int msgID;
    char value[10];
} def_message_struct;

//Default message queue class
class msg_queue {
    public:
        bool send(const def_message_struct& msg);
        bool receive(def_message_struct& msg);
        QueueHandle_t getQueueHandle() const;
        msg_queue();
    private:
        QueueHandle_t queueHandle;
};

class messageAcknowledge {
    public:
        bool addToWaiting(def_message_struct msg, uint8_t addr[6]);
        bool addToFailed(def_message_struct msg, uint8_t addr[6]);
        bool retryInFailed();
        bool isFailedEmpty();
        bool removeFromWaiting(unsigned int msgID);
        bool moveToFailed(unsigned int msgID);
        bool removedFromFailed(unsigned int msgID);
        bool moveAllDelayedInWaiting();
        bool resetFailed();
        int lengthFailed();
        messageAcknowledge();
    private:
        SemaphoreHandle_t awaitingMutex;
        SemaphoreHandle_t failedMutex;
        def_message_struct waitingResponse[MAX_QUEUE_LEN];
        uint8_t waitingAddr[MAX_QUEUE_LEN][6];
        unsigned long timeRecieved[MAX_QUEUE_LEN];
        uint8_t lenWaiting;
        def_message_struct failedDelivery[MAX_QUEUE_LEN];
        uint8_t timesFailed[MAX_QUEUE_LEN];
        uint8_t failedAddr[MAX_QUEUE_LEN][6];
        uint8_t lenFailed;
};

//Default struct used to hold the commands responses and the sensor associated with the commands and responses
typedef struct sensor_definition {
    const char** commands;
    const char** responses;
    const char** values;
    sensor_type sensor;
    const char* name;
};


//Default sensor unit struct which holds pointers to each object that could be defined within it
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


//default communication unit struct
typedef struct communication_unit{
    uint8_t SU_ADDR[6][6];
    sensor_unit_status status[6];
    sensor_type SU_AVLBL_MODULES[6][NUM_OF_SENSORS-1];
    uint8_t SU_NUM_MODULES[6];
    char** names;
    uint8_t numOfSU;
    msg_queue *queue;
    messageAcknowledge *ack;
};
 

//Default sensor definition struct
extern sensor_definition sensors[];

//These two pointers need to be defined in each .ino file to make the library work succesfully
extern sensor_unit *sens_unit_ptr;
extern communication_unit *com_unit_ptr;


void initCU(communication_unit *CU);

bool initSU(sensor_unit *SU);
bool readFromEEPROM(char* nameDest, int destSize, def_message_struct *msg);

int substring(const char* source, int start, int len, char* dest, int bufferLen);

int handleMSG_CU(const def_message_struct &msgRecv);
void handleRequestSU(def_message_struct msgRecv, def_message_struct *response);
void stageForReturn(char* str);

void readAll(sensor_unit *SU);
void clearEEPROM(); 
int sendMessage(uint8_t brdcstAddr[6], uint8_t* msg, int len);
void respondPiRequest(const char* str); 