#include "sensor_units.h"

//Py string word seperator
char pyStrSeper[] = {'|', '\0'};

const char* pyKeywordsArr[][10] = {{"PULL", "PUSH"}, {"TEMP AND HUMID", "GPS", "ALL"}};
const char* pySensorCmds[][10] = {{"TEMP", "HUMID", "ALL"}, {"LAT AND LONG", "ALL"}};

#define PI_SERIAL Serial

const int MAXPYSTRINGLEN = 1000;
int handleMSG_CU(def_message_struct msgRecv, int channel) {
    if (com_unit_ptr == nullptr) {
        return -1;
    }
    int i = 0;
    char returnVal[MAXPYSTRINGLEN];
    returnVal[0] = '\0';
    snprintf(returnVal, sizeof(returnVal), "%s", msgRecv.message);
    
    if (strncmp(msgRecv.message, sens_unit_response[0], MAX_CMD_LENGTH) == 0) {
        snprintf(returnVal, sizeof(returnVal), "%s", pyStrSeper);
        snprintf(returnVal, sizeof(returnVal), "%s", status_strings[(int)msgRecv.values[0]]);
    } else if (strncmp(msgRecv.message, sens_unit_response[1], MAX_CMD_LENGTH) == 0 && com_unit_ptr!=nullptr) {
        for (i = 0; i < msgRecv.numValues; i++) {
            com_unit_ptr->SU_AVLBL_MODULES[msgRecv.channel][i] = (sensor_type)msgRecv.values[i++];
        }
    } else {
        for (i = 0; i < msgRecv.numValues; i++) {
            snprintf(returnVal, MAXPYSTRINGLEN, "%s", pyStrSeper);
            int len = snprintf(NULL, 0, "%f", msgRecv.values[i]);
            char* tempStr = (char*) malloc(sizeof(char)*(len+1));
            snprintf(tempStr, len+1, "%f", msgRecv.values[i]);
            snprintf(returnVal, MAXPYSTRINGLEN, "%s", tempStr);
            free(tempStr);
        }
    }
    stageForReturn(returnVal);
    return 0;
}

inline void stageForReturn(char* str) {
    PI_SERIAL.print(str);
}


void respondPiRequest(const char* str) {
    char* keywordArr[10];
    int len = strlen(str);
    int i = 0;
    int lastInd = 0;
    int keyArrInd = 0;
    for (i = 0; i < len; i++) {
        if (str[i] == pyStrSeper[0] && keyArrInd<10) {
            keywordArr[keyArrInd++] = substring(str, lastInd, (lastInd-i));
            lastInd = i+1;
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
    if (strncmp(keywordArr[0], pyKeywordsArr[0][0], strlen(pyKeywordsArr[0][0])) == 0 && strncmp(keywordArr[1], pyKeywordsArr[1][2], strlen(pyKeywordsArr[1][2])) == 0) {
        while (com_unit_ptr->SU_AVLBL_MODULES[i] != NULL) {
            while (com_unit_ptr->SU_AVLBL_MODULES[i][j] != NULL) {
                while (sensors[com_unit_ptr->SU_AVLBL_MODULES[i][j]].commands[k] != NULL) {
                    memset(&msg, 0, sizeof(msg));
                    msg.message[0] = '\0';
                    strncpy(msg.message, sensors[com_unit_ptr->SU_AVLBL_MODULES[i][j]].commands[k], MAX_MSG_LENGTH);
                    sendMessage(com_unit_ptr->SU_ADDR[i], (uint8_t*)&msg, sizeof(msg));
                    k++;
                }
                j++;
            }
            i++;
        }
    } else if (strncmp(keywordArr[0], pyKeywordsArr[0][0], strlen(pyKeywordsArr[0][0])) == 0) {
        i = 0;
        while (strncmp(keywordArr[1], pyKeywordsArr[1][i], strlen(pyKeywordsArr[1][i])) != 0) {
            i++;
        }
        j = 0;
        while (strncmp(keywordArr[2], pyKeywordsArr[i][j], strlen(pyKeywordsArr[i][j])) != 0) {
            j++;
        }

        k = 0;
        sensor_type sensor = (sensor_type)i;
        int l;
        int m;

        if (strncmp(pySensorCmds[i][j], "ALL", strlen("ALL")) == 0) {
            for (l = 0; l < 6; l++) {
                if (com_unit_ptr->SU_AVLBL_MODULES[l] == NULL) {
                    break;
                }
                for (m = 0; m < 6; m++) {
                    if (com_unit_ptr->SU_AVLBL_MODULES[l][m] == NULL) {
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
            for (l = 0; l < 6; l++) {
                if (com_unit_ptr->SU_AVLBL_MODULES[l] == NULL) {
                    break;
                }
                for (m = 0; m < 6; m++) {
                    if (com_unit_ptr->SU_AVLBL_MODULES[l][m] == NULL) {
                        break;
                    }
                    if (com_unit_ptr->SU_AVLBL_MODULES[l][m] == sensor) {
                        memset(&msg, 0, sizeof(msg));
                        msg.message[0] = '\0';
                        strncpy(msg.message, sensors[i].commands[j], MAX_MSG_LENGTH);
                        sendMessage(com_unit_ptr->SU_ADDR[l], (uint8_t*)&msg, sizeof(msg));
                    }
                }
            }
        }
    } else { //Assuming these are the push commands
        
    }
    i = 0;
    for (i = 0; i < keyArrInd; i++) {
        free(keywordArr[i]);
    }
}




char* substring(const char* source, int start, int len) {
    char* tempStr = (char*)malloc(sizeof(char)*(len+1));
    if (tempStr == NULL) {
        return nullptr;
    }
    strncpy(tempStr, source+start, len);
    tempStr[len] = '\0';
    return tempStr;
}




