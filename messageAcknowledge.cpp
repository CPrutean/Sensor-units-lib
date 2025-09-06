#include "sensor_units.h"

//Make sure to iterate through the full list and nullterminate all the strings at 0 to signify empty values and assign the message id to 0xFF to signify null value
bool messageAcknowledge::addToWaiting(def_message_struct msg, uint8_t addr[6]) {
    if (xSemaphoreTake(awaitingMutex, portMAX_DELAY) == pdTRUE) {
        if (lenWaiting >= MAX_QUEUE_LEN-1) {
            xSemaphoreGive(awaitingMutex);            
            return false;
        }
        memcpy(waitingAddr[lenWaiting], addr, sizeof(addr));
        waitingResponse[lenWaiting++] = msg;
        xSemaphoreGive(awaitingMutex);
        return true;
    }
}

bool messageAcknowledge::isFailedEmpty() {
    return lenFailed <= 0;
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
        for (i = 0; i < lenFailed; i++) {
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

        int j;
        for (j = i; j < lenFailed-1; j++) {
            waitingResponse[j] = waitingResponse[j+1];
            memcpy(waitingAddr[j], waitingAddr[j+1], sizeof(waitingAddr));
        }

        memset(waitingAddr[lenWaiting], 0, sizeof(waitingAddr[lenWaiting]));
        waitingResponse[lenWaiting--] = def_message_struct{{'\0'}, 0, {0.0f, 0.0f, 0.0f, 0.0f}, 0, 0, {0,0,0,0,0,0}, 0};
        failedDelivery[lenFailed++] = msgToBeMoved;

        xSemaphoreGive(awaitingMutex);
        xSemaphoreGive(failedMutex);
        return true;
    }
}

bool messageAcknowledge::removeFromWaiting(unsigned int msgID) {
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
            memcpy(waitingAddr[j], waitingAddr[j+1], sizeof(waitingAddr[j]));
        }
        memset(waitingAddr[lenWaiting], 0, sizeof(waitingAddr[lenWaiting]));
        waitingResponse[lenWaiting--] = def_message_struct{{'\0'}, 0, {0.0f, 0.0f, 0.0f, 0.0f}, 0, 0, {0,0,0,0,0,0}, 0};

        xSemaphoreGive(awaitingMutex);
        return true;
    }
}

bool messageAcknowledge::retryInFailed() {
    if (xSemaphoreTake(failedMutex, portMAX_DELAY) == pdTRUE) {
        int j;
        int i = 0;
        if (!this->isFailedEmpty()) {
            return false;
        }
        while(i < lenFailed) {
            if (sendMessage(failedAddr[i], (uint8_t*)&failedDelivery[i], sizeof(def_message_struct)) == 0) {
                for (j = i; j < lenFailed-1; j++) {
                    failedDelivery[j] = failedDelivery[j+1];
                    memcpy(failedAddr[j], failedAddr[j+1], sizeof(failedAddr[j]));
                }
                failedDelivery[lenFailed] = def_message_struct{{'\0'}, 0, {0.0f, 0.0f, 0.0f, 0.0f}, 0, 0, {0,0,0,0,0,0}, 0};
                memset(failedAddr[lenFailed--], 0, sizeof(failedAddr[lenFailed]));
            } else {
                i++;
            }
        }
        xSemaphoreGive(failedMutex);
        return true;
    }
}

bool messageAcknowledge::removedFromFailed(unsigned int msgID) {
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
        memset(failedAddr[lenFailed--], 0, sizeof(failedAddr[lenFailed]));

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
        xSemaphoreGive(failedMutex);
        return true;
    }
}

int messageAcknowledge::lengthFailed() {
    if (xSemaphoreTake(failedMutex, portMAX_DELAY) == pdTRUE) {
        xSemaphoreGive(failedMutex);
        return lenFailed;
    }
}
