#include "Arduino.h"
#include "freertos/portmacro.h"
#include "sensor_units.h"


constexpr def_message_struct EMPTY_MSG  = {0, BASE_SENS_UNIT, {0.0, 0.0, 0.0}, 0, 0, {'\0'}, DOUBLE_T};


messageAcknowledge::messageAcknowledge() {
  waitingMutex = xSemaphoreCreateRecursiveMutex();
  failedMutex = xSemaphoreCreateRecursiveMutex();
  this->waitingArray= new def_message_struct[this->waitingCapacity]; 
  this->failedArray= new def_message_struct[this->failedCapacity];
  this->waitingSuInd = new uint8_t[this->waitingCapacity];
  this->failedSuInd = new uint8_t[this->failedCapacity];
  this->timesFailed = new uint8_t[this->failedCapacity];
  this->timeRecv = new unsigned long int[this->waitingCapacity];
}


void messageAcknowledge::resizeFailed() {
  xSemaphoreTakeRecursive(this->failedMutex, portMAX_DELAY == pdTRUE);
  def_message_struct* newMsgArr = new def_message_struct[this->failedCapacity*2];
  uint8_t* newSuArr = new uint8_t[this->failedCapacity*2];
  uint8_t* newTimesFailedArr = new uint8_t[this->failedCapacity*2];


  int i;
  for (i = 0; i < this->failedCapacity; i++) {
    newMsgArr[i] = this->failedArray[i];
    newSuArr[i] = this->failedSuInd[i];
    newTimesFailedArr[i] = this->timesFailed[i];
  }
  this->failedCapacity *= 2;
  this->failedArray = newMsgArr;
  this->failedSuInd = newSuArr;
  this->timesFailed = newTimesFailedArr;
  xSemaphoreGiveRecursive(this->failedMutex);
}

void messageAcknowledge::resizeWaiting() {
  xSemaphoreTakeRecursive(this->waitingMutex, portMAX_DELAY == pdTRUE);
  def_message_struct* newMsgArr = new def_message_struct[this->waitingCapacity*2];
  uint8_t* newSuArr = new uint8_t[this->waitingCapacity*2];
  unsigned long int *newTimeRecv = new unsigned long int[this->waitingCapacity*2];

  int i;
  for (i = 0; i < this->waitingCapacity; i++) {
    newMsgArr[i] = this->waitingArray[i];
    newSuArr[i] = this->waitingSuInd[i];
    newTimeRecv[i] = this->timeRecv[i];
  }
  this->waitingCapacity *= 2;

  this->waitingArray = newMsgArr;
  this->waitingSuInd = newSuArr;
  xSemaphoreGiveRecursive(this->waitingMutex);

}


bool messageAcknowledge::addToWaiting(def_message_struct msg,uint8_t suInd) {
  xSemaphoreTakeRecursive(this->waitingMutex, portMAX_DELAY == pdTRUE);

  if (this->waitingLen+1 >= this->waitingCapacity) {
    this->resizeWaiting();
  }
  this->waitingArray[this->waitingLen] = msg;
  this->waitingSuInd[this->waitingLen] = suInd;
  this->timeRecv[this->waitingLen] = millis();
  this->waitingLen++;
  return true;
  xSemaphoreGiveRecursive(this->waitingMutex);
}

bool messageAcknowledge::addToFailed(def_message_struct msg, uint8_t suInd) {
  xSemaphoreTakeRecursive(this->failedMutex, portMAX_DELAY == pdTRUE);

  if (this->failedLen+1 >= this->failedCapacity) {
    this->resizeFailed();
  }
  this->failedArray[this->failedLen] = msg;
  this->failedSuInd[this->failedLen] = suInd;
  this->failedLen++;
  return true;
  xSemaphoreGiveRecursive(this->failedMutex);
}


bool messageAcknowledge::retryInFailed(communication_unit &CU) {
  xSemaphoreTakeRecursive(this->failedMutex, portMAX_DELAY == pdTRUE);
  int i;
  for (i = 0; i < this->failedLen; i++) {
    CU.sendMessage(this->failedArray[i], this->failedSuInd[i]);
    this->timesFailed[i]++;
    if (this->timesFailed[i] > 3) {
      CU.sensorUnitStatus[this->failedSuInd[i]] = OFFLINE;
      this->removedFromFailed(this->failedArray[i].msgID);
    }
  }
  xSemaphoreGiveRecursive(this->failedMutex);
}

bool messageAcknowledge::isFailedEmpty() {
  xSemaphoreTakeRecursive(this->failedMutex, portMAX_DELAY == pdTRUE);
  bool condition = this->failedLen <= 0;
  xSemaphoreGiveRecursive(this->failedMutex);
  return condition;
}

bool messageAcknowledge::removeFromWaiting(unsigned long int msgID) {
  xSemaphoreTakeRecursive(this->waitingMutex, portMAX_DELAY == pdTRUE);
  bool found = false; 
  int i;
  for (i = 0; i < this->waitingLen; i++) {
    if (this->waitingArray->msgID == msgID) {
      found = true;
      break;
    }
  }

  if (!found) {
    return false;
  }

  int ind = i;
  for (i = ind; i < this->waitingLen-1; i++) {
    this->waitingArray[i] = this->waitingArray[i+1];
    this->waitingSuInd[i] = this->waitingSuInd[i+1];
    this->timeRecv[i] = this->timeRecv[i+1];
  }
  this->waitingArray[waitingLen] = EMPTY_MSG; 
  this->waitingSuInd[waitingLen] = -1;
  this->timeRecv[waitingLen] = 0;
  this->waitingLen--;
  xSemaphoreGive(this->waitingMutex);
  return true;
}

bool messageAcknowledge::removedFromFailed(unsigned long int msgID) {
  xSemaphoreTakeRecursive(this->failedMutex, portMAX_DELAY == pdTRUE);
  bool found = false; 
  int i;
  for (i = 0; i < this->failedLen; i++) {
    if (this->failedArray[i].msgID == msgID) {
      found = true;
      break;
    }
  }

  if (!found) {
    return false;
  }

  int ind = i;
  for (i = ind; i < this->failedLen-1; i++) {
    this->failedArray[i] = this->failedArray[i+1];
    this->failedSuInd[i] = this->failedSuInd[i+1];
    this->timesFailed[i] = this->timesFailed[i+1];
  }
  this->failedArray[this->failedLen] = EMPTY_MSG; 
  this->failedSuInd[this->failedLen] = -1;
  this->timesFailed[this->failedLen] = -1;
  this->failedLen--;

  xSemaphoreGive(this->failedMutex);
  return true;

}

bool messageAcknowledge::moveToFailed(unsigned long int msgID) {
  xSemaphoreTakeRecursive(this->failedMutex, portMAX_DELAY == pdTRUE);  
  xSemaphoreTakeRecursive(this->waitingMutex, portMAX_DELAY == pdTRUE);

  bool found = false;
  int i;
  for (i = 0; i < this->failedLen; i++) {
    if (this->failedArray[i].msgID == msgID) {
      found = true;      
      break;
    }
  }

  if (!found) {
    return false;
  }
  
  int ind = i;
  this->addToWaiting(this->failedArray[ind], this->failedSuInd[ind]);
  this->removedFromFailed(msgID);
  return true;

  xSemaphoreGive(this->waitingMutex);
  xSemaphoreGive(this->failedMutex);
}

bool messageAcknowledge::moveAllDelayedInWaiting() {
  xSemaphoreTakeRecursive(this->failedMutex, portMAX_DELAY == pdTRUE);  
  xSemaphoreTakeRecursive(this->waitingMutex, portMAX_DELAY == pdTRUE);
  const unsigned long int currMaxDelay = 100000;
  
  unsigned long int currTime = millis();
  int i;
  for (i = 0; i < this->waitingLen; i++) {
    if (currTime-this->timeRecv[i] > currMaxDelay) {
      this->addToFailed(this->waitingArray[i], this->waitingSuInd[i]);
      this->removeFromWaiting(this->waitingArray[i].msgID);
    }
  }
  xSemaphoreGive(this->waitingMutex);
  xSemaphoreGive(this->failedMutex);
}

int messageAcknowledge::sizeFailed() {
  xSemaphoreTakeRecursive(this->failedMutex, portMAX_DELAY == pdTRUE);
  int len = this->failedLen;
  xSemaphoreGive(this->failedMutex);
  return len;
}

int messageAcknowledge::sizeWaiting() {
  xSemaphoreTakeRecursive(this->waitingMutex, portMAX_DELAY == pdTRUE);
  int len = this->waitingLen;
  xSemaphoreGive(this->waitingLen);
  return len;

}

int messageAcknowledge::capacityFailed() {
  xSemaphoreTakeRecursive(this->failedMutex, portMAX_DELAY == pdTRUE);
  int len = this->failedCapacity;
  xSemaphoreGive(this->failedMutex);
  return len;
}

int messageAcknowledge::capacityWaiting() {
  xSemaphoreTakeRecursive(this->waitingMutex, portMAX_DELAY == pdTRUE);
  int len = this->waitingCapacity;
  xSemaphoreGive(this->waitingMutex);
  return len;
}
