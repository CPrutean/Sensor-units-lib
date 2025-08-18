#include "sensor_units.h"

//Py string word seperator
char pyStrSeper[] = {'|', '\0'};

const char* pyKeywordsArr[][10] = {{"PULL", "PUSH", NULL}, {"TEMP AND HUMID", "GPS", "ALL", NULL}};
const char* pySensorCmds[][10] = {{"TEMP", "HUMID", "ALL", NULL}, {"LAT AND LONG", "ALL", NULL}, {"NEW NAME"}};


const int MAXPYSTRINGLEN = 1000;



inline void stageForReturn(char* str) {
    Serial.print(str);
}


int substring(const char* source, int start, int len, char* dest, int bufferLen) {
    if (dest == NULL || len > bufferLen) {
        return -1;
    }
    strncpy(dest, source+start, len);
    dest[len] = '\0';
    return 0;
}

int handleMSG_CU(def_message_struct msgRecv, int channel) {
    if (com_unit_ptr == nullptr) {
        #ifdef DEBUG
        Serial.print("Com_unit_ptr was never initialized");
        #endif
        return -1;
    }
    int i = 0;
    char returnVal[MAXPYSTRINGLEN];
    returnVal[0] = '\0';
    snprintf(returnVal, sizeof(returnVal), "%s", msgRecv.message);
    
    if (strncmp(msgRecv.message, sens_unit_response[0], MAX_CMD_LENGTH) == 0) {
        strncat(returnVal, pyStrSeper, sizeof(returnVal) - strlen(returnVal) - 1);
        strncat(returnVal, status_strings[(int)msgRecv.values[0]], sizeof(returnVal) - strlen(returnVal) - 1);
    } else if (strncmp(msgRecv.message, sens_unit_response[1], MAX_CMD_LENGTH) == 0 && com_unit_ptr!=nullptr) {
        for (i = 0; i < msgRecv.numValues; i++) {
            com_unit_ptr->SU_AVLBL_MODULES[msgRecv.channel][i] = (sensor_type)msgRecv.values[i++];
            com_unit_ptr->SU_NUM_MODULES[msgRecv.channel]++;
        }
    } else {
        for (i = 0; i < msgRecv.numValues; i++) {
            strncat(returnVal, pyStrSeper, MAXPYSTRINGLEN);
            int len = snprintf(NULL, 0, "%f", msgRecv.values[i]);
            char* tempStr = (char*) malloc(sizeof(char)*(len+1));
            if (tempStr != NULL) {
                snprintf(tempStr, len+1, "%f", msgRecv.values[i]);
                strncat(returnVal, tempStr, MAXPYSTRINGLEN);
            } else {
                #ifdef DEBUG
                Serial.print("Malloc failed when parsing floats");
                #endif
            }
            free(tempStr);
        }
    }
    stageForReturn(returnVal);
}


const int maxKeywordLen = 30;
void respondPiRequest(const char* str) {
    #ifdef DEBUG
    Serial.print("MESSAGE RECIEVED");
    #endif
    char keywordArr[10][30];
    int keywordArrLen[10];
    int len = strlen(str);
    int i = 0;
    int lastInd = 0;
    int keyArrInd = 0;
    for (i = 0; i < len; i++) {
        if (str[i] == pyStrSeper[0] && keyArrInd<10) {
            substring(str, lastInd, (i - lastInd), keywordArr[keyArrInd], maxKeywordLen);
            if (keywordArr[keyArrInd] != nullptr) { // Always check malloc result
                keywordArrLen[keyArrInd] = strlen(keywordArr[keyArrInd]);
                #ifdef DEBUG
                Serial.println(keywordArr[keyArrInd]);
                #endif
                keyArrInd++;
            }
            lastInd = i+1;
        }
    }
    if (lastInd <= len && keyArrInd < 10) {
        substring(str, lastInd, len - lastInd, keywordArr[keyArrInd], maxKeywordLen);
        if (keywordArr[keyArrInd] != nullptr) {
            keywordArrLen[keyArrInd] = strlen(keywordArr[keyArrInd]);
            keyArrInd++;
        }
    }

    
    //The first index is always going to determine push or pull in a command
    //The second specifies what sensor were pulling or pushing to whether thats the temperature sensor or the gps

    //The raspberry pi shouldnt be aware of how many sensors units have which sensors
    //Instead what will happen is itll recieve multiple readings and then average the result of all of them

    //TODO IMPLEMENT PUSH FUNCTIONALITY(currently no SU units require push functionality to be implemented later with more sensors being added)

    //If recieved push all send requests to all available sensors 
    i = 0;
    int j = 0;
    int k = 0;
    def_message_struct msg;
    if (strncmp(keywordArr[0], pyKeywordsArr[0][0], keywordArrLen[0]) == 0 && strncmp(keywordArr[1], pyKeywordsArr[1][2], keywordArrLen[1]) == 0) {
        for (i = 0; i < com_unit_ptr->numOfSU; i++) {
            for (j = 0; j < com_unit_ptr->SU_NUM_MODULES[i]; j++) {
                while (sensors[com_unit_ptr->SU_AVLBL_MODULES[i][j]].commands[k] != NULL) {
                    memset(&msg, 0, sizeof(msg));
                    msg.message[0] = '\0';
                    strncpy(msg.message, sensors[com_unit_ptr->SU_AVLBL_MODULES[i][j]].commands[k], MAX_MSG_LENGTH);
                    sendMessage(com_unit_ptr->SU_ADDR[i], (uint8_t*)&msg, sizeof(msg));
                    k++;
                }
                k = 0;
            }
        }
    } else if (strncmp(keywordArr[0], pyKeywordsArr[0][0], keywordArrLen[0]) == 0) {
        i = 0;
        while (pyKeywordsArr[1][i] != NULL && strncmp(keywordArr[1], pyKeywordsArr[1][i], keywordArrLen[1]) != 0) {
            i++;
        }
        if (pyKeywordsArr[1][i] == NULL) {
            Serial.println("pyKeywordsArr was null terminating");
            return;
        }
        j = 0;
        while (pyKeywordsArr[i][j] != NULL && strncmp(keywordArr[2], pyKeywordsArr[i][j], keywordArrLen[2]) != 0) {
            j++;
        }
        if (pyKeywordsArr[i][j] == NULL) {
            Serial.println("pyKeywordsArr was null terminating");
            return;
        }

        k = 0;
        sensor_type sensor = (sensor_type)i;
        int l;
        int m;
        
        if (strncmp(pySensorCmds[i][j], "ALL", 3) == 0) {
            for (l = 0; l < com_unit_ptr->numOfSU; l++) {
                for (m = 0; m < com_unit_ptr->SU_NUM_MODULES[l]; m++) {
                    if (com_unit_ptr->SU_AVLBL_MODULES[l][m] == NUM_OF_SENSORS) {
                        break;
                    }
                    if (com_unit_ptr->SU_AVLBL_MODULES[l][m] == sensor) {
                        while (sensors[i].commands[k] != NULL)  {
                            memset(&msg, 0, sizeof(msg));
                            msg.message[0] = '\0';
                            msg.strlen = snprintf(msg.message, MAX_MSG_LENGTH, "%s", sensors[i].commands[k]);
                            sendMessage(com_unit_ptr->SU_ADDR[l], (uint8_t*)&msg, sizeof(msg));
                            k++;
                        }
                        break;
                    }
                }
            }
        } else {
            for (l = 0; l < com_unit_ptr->numOfSU; l++) {
                for (m = 0; m < com_unit_ptr->SU_NUM_MODULES[l]; m++) {
                    if (com_unit_ptr->SU_AVLBL_MODULES[l][m] == sensor) {
                        memset(&msg, 0, sizeof(msg));
                        msg.message[0] = '\0';
                        msg.strlen = snprintf(msg.message, MAX_MSG_LENGTH, "%s", sensors[i].commands[j]);
                        sendMessage(com_unit_ptr->SU_ADDR[l], (uint8_t*)&msg, sizeof(msg));
                    }
                }
            }
        }
    } else { //Assuming these are the push commands
        
    }
    i = 0;
}







