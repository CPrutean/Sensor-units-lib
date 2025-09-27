#include "sensor_units.h"

//Make sure to iterate through the full list and nullterminate all the strings at 0 to signify empty values and assign the message id to 0xFF to signify null value
bool messageAcknowledge::addToWaiting(def_message_struct msg, uint8_t addr[6]) {
    if (xSemaphoreTake(awaitingMutex, portMAX_DELAY) == pdTRUE) {
        if (lenWaiting >= MAX_QUEUE_LEN-1) {
            xSemaphoreGive(awaitingMutex);            
            return false;
        }
        memcpy(waitingAddr[lenWaiting], addr, sizeof(waitingAddr[0]));
        timeReceived[lenWaiting] = millis();
        waitingResponse[lenWaiting++] = msg;
        xSemaphoreGive(awaitingMutex);
        return true;
    }
}

bool messageAcknowledge::isFailedEmpty() {
    if (xSemaphoreTake(failedMutex, portMAX_DELAY) == pdTRUE) {
        bool cond = lenFailed <= 0;
        xSemaphoreGive(failedMutex);
        return cond;
    }
    return false;
}

bool messageAcknowledge::moveToFailed(unsigned int msgID) {
    if (xSemaphoreTake(failedMutex, portMAX_DELAY) == pdTRUE && xSemaphoreTake(awaitingMutex, portMAX_DELAY) == pdTRUE) {
        if (lenFailed <= 0) {
            xSemaphoreGive(failedMutex);
            xSemaphoreGive(awaitingMutex);
            return false;
        }

        if (lenFailed == MAX_QUEUE_LEN-1) {
            xSemaphoreGive(failedMutex);
            xSemaphoreGive(awaitingMutex);
            return false;
        }
        bool msgFound = false;
        int i;
        for (i = 0; i < lenWaiting; i++) {
            if (waitingResponse[i].message[0] != '\0' && waitingResponse[i].msgID == msgID) {
                msgFound = true;
                break;
            }
        }
        def_message_struct msgToBeMoved;
        msgToBeMoved = waitingResponse[i];

        if (!msgFound) {
            xSemaphoreGive(failedMutex);
            xSemaphoreGive(awaitingMutex);
            return false;    
        }

        memcpy(failedAddr[lenFailed], waitingAddr[i], sizeof(failedAddr[0]));
        failedDelivery[lenFailed] = msgToBeMoved;
        timesFailed[lenFailed] = 0;
        lenFailed++;

        int j;
        for (j = i; j < lenFailed-1; j++) {
            waitingResponse[j] = waitingResponse[j+1];
            memcpy(waitingAddr[j], waitingAddr[j+1], sizeof(waitingAddr[0]));
        }

        memset(waitingAddr[lenWaiting], 0, sizeof(waitingAddr[lenWaiting]));
        waitingResponse[lenWaiting] = def_message_struct{{'\0'}, 0, {0.0f, 0.0f, 0.0f, 0.0f}, 0, 0, {0,0,0,0,0,0}, 0};
        lenWaiting--;
        xSemaphoreGive(awaitingMutex);
        xSemaphoreGive(failedMutex);
        return true;
    }
}

bool messageAcknowledge::removeFromWaiting(unsigned int msgID) {
    #ifdef DEBUG
    Serial.println("Attempting to remove from waiting queue");
    #endif
    if (xSemaphoreTake(awaitingMutex, portMAX_DELAY) == pdTRUE) {
        int i;
        bool msgFound = false;
        for (i = 0; i < lenWaiting; i++) {
            if (waitingResponse[i].msgID == msgID) {
                msgFound = true;
                break;
            }
        }

        if (!msgFound) {
            xSemaphoreGive(awaitingMutex);
            return false;
        }
        int j;
        for (j = i; j < lenWaiting-1; j++) {
            waitingResponse[j] = waitingResponse[j+1];
            timeReceived[j] = timeReceived[j+1];
            memcpy(waitingAddr[j], waitingAddr[j+1], sizeof(waitingAddr[j]));
        }
        memset(waitingAddr[lenWaiting], 0, sizeof(waitingAddr[lenWaiting]));
        timeReceived[lenWaiting] = 0;
        waitingResponse[lenWaiting] = def_message_struct{{'\0'}, 0, {0.0f, 0.0f, 0.0f, 0.0f}, 0, 0, {0,0,0,0,0,0}, 0};
        lenWaiting--;
        xSemaphoreGive(awaitingMutex);
        return true;
    }
}

bool messageAcknowledge::retryInFailed() {
    if (xSemaphoreTake(failedMutex, portMAX_DELAY) == pdTRUE) {
        int j;
        int i = 0;
        if (lenFailed <= 0) {
            xSemaphoreGive(failedMutex);
            return false;
        }
        
        for (i = 0; i < lenFailed; i++) {
            timesFailed[i]++;
            if (timesFailed[i] > 3) {
                char tempStr[MAX_MSG_LENGTH];
                tempStr[0] = '\0';
                snprintf(tempStr, sizeof(tempStr), "%s", "MESSAGE FAILED TO SEND: ");
                strncat(tempStr, failedDelivery[i].message, sizeof(tempStr)-strlen(tempStr)-1);
                stageForReturn(tempStr);
                tempStr[0] = '\0';

                int k;
                bool macFound = true;
                for (j = 0; j < com_unit_ptr->numOfSU; j++) {
                    macFound = true;
                    for (k = 0; k < 6; k++) {
                        if (failedAddr[i][k] != com_unit_ptr->SU_ADDR[j][k]) {
                            macFound = false;
                            break;
                        }
                    }
                    if (macFound) {
                        break;
                    }
                }

                if (!macFound) {
                    stageForReturn("Couldnt find mac when checking for com_unit_ptr index");
                    return false;
                }

                snprintf(tempStr, sizeof(tempStr), "%s", "Status|Offline|");
                char indStr[2];
                indStr[0] = (char)(j+(int)'0');
                indStr[1] = '\0';

                strncat(tempStr, indStr, sizeof(tempStr)-strlen(tempStr)-1);
                stageForReturn(tempStr);

                for (j = i; j < lenFailed-1; j++) {
                    failedDelivery[j] = failedDelivery[j+1];
                    memcpy(failedAddr[j], failedAddr[j+1], sizeof(failedAddr[j]));
                    timesFailed[j] = timesFailed[j+1];
                }
                failedDelivery[lenFailed] = def_message_struct{{'\0'}, 0, {0.0f, 0.0f, 0.0f, 0.0f}, 0, 0, {0,0,0,0,0,0}, 0};
                memset(failedAddr[lenFailed], 0, sizeof(failedAddr[lenFailed]));
                timesFailed[lenFailed] = 0;
                lenFailed--;
                i--;
            } else {
                sendMessage(failedAddr[i], (uint8_t*)&failedDelivery[i], sizeof(def_message_struct));
            }
        }
        xSemaphoreGive(failedMutex);
        return true;
    }
}

bool messageAcknowledge::removedFromFailed(unsigned int msgID) {
    #ifdef DEBUG
    Serial.println("Attempting to remove from failed queue"); 
    #endif
    if (xSemaphoreTake(failedMutex, portMAX_DELAY) == pdTRUE) {
        int i;
        bool msgFound = false;
        for (i = 0; i < lenFailed; i++) {
            if (failedDelivery[i].msgID == msgID) {
                msgFound = true;
                break;
            }
        }
        if (!msgFound) {
            xSemaphoreGive(failedMutex);
            return false;
        }

        int j;
        for (j = i; j < lenFailed-1; j++) {
            failedDelivery[j] = failedDelivery[j+1];
            memcpy(failedAddr[j], failedAddr[j+1], sizeof(failedAddr[j]));
        }
        failedDelivery[lenFailed] = def_message_struct{{'\0'}, 0, {0.0f, 0.0f, 0.0f, 0.0f}, 0, 0, {0,0,0,0,0,0}, 0};
        memset(failedAddr[lenFailed], 0, sizeof(failedAddr[lenFailed]));
        lenFailed--;
        xSemaphoreGive(failedMutex);
        return true;
    }
}

bool messageAcknowledge::addToFailed(def_message_struct msg, uint8_t addr[6]) {
    if (xSemaphoreTake(failedMutex, portMAX_DELAY) == pdTRUE) {
        failedDelivery[lenFailed] = msg;
        memcpy(failedAddr[lenFailed++], addr, sizeof(failedAddr[lenFailed]));
        xSemaphoreGive(failedMutex);
        return true;
    }
}

bool messageAcknowledge::resetFailed() {
    if (xSemaphoreTake(failedMutex, portMAX_DELAY) == pdTRUE) {
        int i;
        for (i = 0; i < MAX_QUEUE_LEN; i++) {
            failedDelivery[i] = def_message_struct{{'\0'}, 0, {0.0f, 0.0f, 0.0f, 0.0f}, 0, 0, {0,0,0,0,0,0}, 0};
        }
        memset(failedAddr, 0, sizeof(failedAddr));
        lenFailed = 0;
        xSemaphoreGive(failedMutex);
        return true;
    }
}

int messageAcknowledge::lengthFailed() {
    if (xSemaphoreTake(failedMutex, portMAX_DELAY) == pdTRUE) {
        int len = lenFailed;
        xSemaphoreGive(failedMutex);
        return len;
    }
}

bool messageAcknowledge::moveAllDelayedInWaiting() {
    #ifdef DEBUG
    Serial.println("Moving delayed messages to failed");
    #endif
    if (xSemaphoreTake(failedMutex, portMAX_DELAY) == pdTRUE && xSemaphoreTake(awaitingMutex, portMAX_DELAY) == pdTRUE) {
        unsigned long currMillis = millis();
        int i = 0;
        int j;
        while (i < lenWaiting && lenFailed < MAX_QUEUE_LEN) {
            if (currMillis-timeReceived[i] >= 10000) {
                failedDelivery[lenFailed] = waitingResponse[i];
                memcpy(failedAddr[lenFailed], waitingAddr[i], sizeof(failedAddr[0]));
                lenFailed++;
                for (j = 0; j < lenWaiting-1; j++) {
                    waitingResponse[j] = waitingResponse[j+1];
                    memcpy(waitingAddr[j], waitingAddr[j+1], sizeof(waitingAddr[j]));
                }
                waitingResponse[lenWaiting] = def_message_struct{{'\0'}, 0, {0.0f, 0.0f, 0.0f, 0.0f}, 0, 0, {0,0,0,0,0,0}, 0};
                memset(waitingAddr[lenWaiting], 0, sizeof(waitingAddr[lenWaiting]));
                lenWaiting--;
            } else {
                i++;
            }
        }
        xSemaphoreGive(failedMutex);
        xSemaphoreGive(awaitingMutex);
        return true;
    }
}

messageAcknowledge::messageAcknowledge() {
    awaitingMutex = xSemaphoreCreateMutex();
    failedMutex = xSemaphoreCreateMutex();
    lenWaiting = 0;
    lenFailed = 0;
}
