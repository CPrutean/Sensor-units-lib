#ifndef __MY_SENSOR_LIB
#define __MY_SENSOR_LIB

// Uncomment this header when compiling tests
// #define DEBUG 0
#include "core/gps_sensor.h"
#include "core/motion_sensor.h"
#include "core/temperature_sensor.h"
#ifndef DEBUG
#include <Arduino.h>
#include <EEPROM.h>
#include <LCD_I2C.h>
#include <WiFi.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <esp_now.h>
#include <esp_wifi.h>
#endif

#define GPS_BAUD 9600
#define MAX_MSG_LENGTH 100
#define MAX_CMD_LENGTH 32
#define EEPROM_SIZE 512
#define MAX_READINGS 80
#define MAX_NAME_LEN 30
#define MAX_CMDS 32
#define MAX_QUEUE_LEN 25
#define MAX_ESP32_PEERS 10

// Default defined in sensor_units_var.cpp
extern const char *temp_sensor_cmds[];
extern const char *temp_sensor_responses[];

extern const char *gps_sensor_cmds[];
extern const char *gps_sensor_responses[];

extern const char *sens_unit_msgs[];
extern const char *sens_unit_response[];

extern const char *motion_unit_msgs[];
extern const char *motion_unit_response[];

extern const char *sens_unit_strings[];
extern const char *status_strings[];

enum sensor_type {
  TEMP_AND_HUMID = 0,
  GPS,
  MOTION_SENSOR,
  BASE_SENS_UNIT,
  NUM_OF_SENSORS
};
enum sensor_unit_status { ONLINE = 0, ERROR, OFFLINE };
enum def_message_struct_DATA_TYPES { DOUBLE_T, STRING_T };
// The default messages sent to and from the communication and sensor units
typedef struct def_message_struct {
  int command_ind;
  sensor_type sensor_req;
  double values[NUM_OF_SENSORS - 1];
  uint8_t numValues;
  unsigned long msgID;
  char message[30] = {'\0'};
  def_message_struct_DATA_TYPES type = DOUBLE_T;
} def_message_struct;
// Default message queue class
class msg_queue {
public:
  bool send(const def_message_struct &msg);
  bool receive(def_message_struct &msg);
  QueueHandle_t getQueueHandle() const;
  msg_queue();

private:
  QueueHandle_t queueHandle;
};

class messageAcknowledge {
public:
  bool addToWaiting(def_message_struct msg, uint8_t addr[6],
                    bool nested = false);
  bool addToFailed(def_message_struct msg, uint8_t addr[6],
                   bool nested = false);
  bool retryInFailed();
  bool isFailedEmpty(bool nested = false);
  bool removeFromWaiting(unsigned int msgID, bool nested = false);
  bool moveToFailed(unsigned int msgID, bool nested = false);
  bool removedFromFailed(unsigned int msgID, bool nested = false);
  bool moveAllDelayedInWaiting();
  bool resetFailed(bool nested = false);
  int lengthFailed();
  messageAcknowledge();

private:
  SemaphoreHandle_t awaitingMutex;
  SemaphoreHandle_t failedMutex;
  def_message_struct waitingResponse[MAX_QUEUE_LEN];
  uint8_t waitingAddr[MAX_QUEUE_LEN][6];
  unsigned long timeReceived[MAX_QUEUE_LEN];
  uint8_t lenWaiting;
  def_message_struct failedDelivery[MAX_QUEUE_LEN];
  uint8_t timesFailed[MAX_QUEUE_LEN];
  uint8_t failedAddr[MAX_QUEUE_LEN][6];
  uint8_t lenFailed;
};

// Default struct used to hold the commands responses and the sensor associated
// with the commands and responses
typedef struct sensor_definition {
  const char **commands;
  const char **responses;
  const char **values;
  sensor_type sensor;
  const char *name;
} sensor_definition;

class sensor_unit {
public:
  sensor_unit(sensor_type *modules = nullptr, uint8_t numSensors = 0,
              char *nameIn = nullptr,
              temperature_sensor *tempSensorIn = nullptr,
              gps_sensor *gpsIn = nullptr, motion_sensor *motionIn = nullptr);
  void sendMessage(def_message_struct msg);
  void handleMsg(def_message_struct msgIn);
  void initESP_NOW(uint8_t *cuAddrIn, const char *PMK_KEY, const char *LMK_KEY);

private:
  sensor_type modules[NUM_OF_SENSORS - 1];
  uint8_t moduleCount;
  temperature_sensor tempSensor;
  gps_sensor gpsSensor;
  motion_sensor motionSensor;
  uint8_t cuAddr[6];
  msg_queue messageQueue;
  char name[MAX_NAME_LEN];
  esp_now_peer_info_t cuPeerInf;
  bool nameFromEEPROM = false;
};

class communication_unit {
public:
  unsigned long sendMessage(def_message_struct msgOut, int SUIND);
  communication_unit();
  void handleMsg(def_message_struct msgIn);
  void handleServerRequest(char *buffer, int sizeOfBuffer);
  void initESP_NOW(uint8_t **suMac, uint8_t numOfSU, const char *PMK_KEY_IN,
                   const char *LMK_KEY_IN);

private:
  uint8_t sens_unit_addresses[6][6];
  uint8_t numOfSU = 0;
  char names[6][6];
  sensor_unit_status sensorUnitStatus[6];
  sensor_type availableModules[6][NUM_OF_SENSORS - 1];
  msg_queue messageQueue;
  messageAcknowledge acknowledgementQueue;
  esp_now_peer_info_t suPeerInf[6];
  static unsigned long msgID;
};

extern communication_unit *com_unit_ptr;
extern sensor_unit *sens_unit_ptr;

// Default sensor definition struct
extern sensor_definition sensors[];
void initCU(communication_unit *CU);

bool initSU(sensor_unit *SU);
bool readFromEEPROM(char *nameDest, int destSize, def_message_struct *msg);

int substring(const char *source, int start, int len, char *dest,
              int bufferLen);

int handleMSG_CU(const def_message_struct &msgRecv);
void handleRequestSU(def_message_struct msgRecv, def_message_struct *response);
void stageForReturn(char *str);

void readAll(sensor_unit *SU);
void clearEEPROM();

void onReceiveCBSU(uint8_t *macAddr, uint8_t *data, int size);
void onReceiveCBCU(uint8_t *macAddr, uint8_t *data, int size);

void onSendCBSU(uint8_t *macAddr, esp_now_send_status_t status);
void onSendCBCU(uint8_t *macAddr, esp_now_send_status_t status);

#endif
