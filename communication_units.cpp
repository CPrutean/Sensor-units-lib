#include "sensor_units.h"

//Py string word seperator
char pyStrSeper[] = {'|', '\0'};



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

const int MAXPYSTRINGLEN = 1000;
int handleMSG_CU(const def_message_struct& msgRecv, int SUInd) {
    if (com_unit_ptr == nullptr) {
        #ifdef DEBUG
        Serial.print("Com_unit_ptr was never initialized");
        #endif
        return -1;
    }

    char returnVal[MAXPYSTRINGLEN];

    snprintf(returnVal, sizeof(returnVal), "%s", msgRecv.message);

    if (strncmp(msgRecv.message, sens_unit_response[0], MAX_CMD_LENGTH) == 0) {
        strncat(returnVal, pyStrSeper, sizeof(returnVal) - strlen(returnVal) - 1);

        int status_index = (int)msgRecv.values[0];
        if (status_index >= 0 && status_index < 3) {
            strncat(returnVal, status_strings[status_index], sizeof(returnVal) - strlen(returnVal) - 1);
        } else {
            strncat(returnVal, "INVALID_STATUS", sizeof(returnVal) - strlen(returnVal) - 1);
        }
    } else if (strncmp(msgRecv.message, sens_unit_response[1], strlen(sens_unit_response[1])) == 0) {
        for (int i = 0; i < msgRecv.numValues; i++) {
            com_unit_ptr->SU_AVLBL_MODULES[msgRecv.suInd][i] = (sensor_type)msgRecv.values[i];
            com_unit_ptr->SU_NUM_MODULES[msgRecv.suInd]++;
        }
        
    } else {
        for (int i = 0; i < msgRecv.numValues; i++) {
            strncat(returnVal, pyStrSeper, sizeof(returnVal) - strlen(returnVal) - 1);

            int len = snprintf(NULL, 0, "%f", msgRecv.values[i]);
            char* tempStr = (char*) malloc(sizeof(char) * (len + 1));

            if (tempStr != NULL) {
                snprintf(tempStr, len + 1, "%f", msgRecv.values[i]);
                strncat(returnVal, tempStr, sizeof(returnVal) - strlen(returnVal) - 1);
                free(tempStr);
            } else {
                #ifdef DEBUG
                Serial.print("Malloc failed when parsing floats");
                #endif
            }
        }
    }
    stageForReturn(returnVal);
    return 0; 
}


const int maxKeywordLen = 30;


//the PI will send an index for what corresponding SU or all at the end of the message to correspond to which SU it is pulling from
void respondPiRequest(const char* str) {
    #ifdef DEBUG
    Serial.print("MESSAGE RECIEVED");
    #endif
    int len = strlen(str);
    char keywords[10][MAX_CMD_LENGTH];
    int i;
    int lastInd = 0;
    int keyArrInd = 0;
    for (i = 0; i < len; i++) {
        if (str[i] == pyStrSeper[0]) {
            substring(str, lastInd, i-lastInd, keywords[keyArrInd], MAX_CMD_LENGTH);
            lastInd = i+1;
        }
    }
    //Case for pulling all available data from all available sensor unitsz
    if (keywords[0] != NULL && keywords[1] != NULL && strncmp(keywords[0], "PULL", strlen("PULL")) == 0 && strncmp(keywords[1], "ALL", strlen("ALL")) == 0) {
        int j;
        int k;
        int l = 0;
        def_message_struct msg;
        memset(&msg, 0, sizeof(msg));
        for (i = 0; i < com_unit_ptr->numOfSU; i++) {
            
            for (j = 0; j < 3; j++) {
                msg.message[0] = '\0';
                strncpy(msg.message, sensors[BASE_SENS_UNIT].commands[j], MAX_MSG_LENGTH);
                msg.message[strlen(sensors[BASE_SENS_UNIT].commands[j])] = '\0';
                sendMessage(com_unit_ptr->SU_ADDR[i], (uint8_t*)&msg, sizeof(msg));
            }
            for (k = 0; k < com_unit_ptr->SU_NUM_MODULES[i]; k++) {
                switch(com_unit_ptr->SU_AVLBL_MODULES[i][k]) {
                    case (TEMP_AND_HUMID):
                        while (sensors[TEMP_AND_HUMID].commands[l] != NULL) {
                            msg.message[0] = '\0';
                            strncpy(msg.message, sensors[TEMP_AND_HUMID].commands[l], MAX_MSG_LENGTH);
                            sendMessage(com_unit_ptr->SU_ADDR[i], (uint8_t*)&msg, sizeof(msg));
                            l++;
                        }
                        break;
                    case (GPS):
                        while (sensors[GPS].commands[l] != NULL) {
                            msg.message[0] = '\0';
                            strncpy(msg.message, sensors[TEMP_AND_HUMID].commands[l], MAX_MSG_LENGTH);
                            sendMessage(com_unit_ptr->SU_ADDR[i], (uint8_t*)&msg, sizeof(msg));
                            l++;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    } else {
        int i;
        int j = 0;
        bool commandFound = false;
        for (i = 0; i < NUM_OF_SENSORS; i++) {
            j = 0;
            while (sensors[i].commands[j] != NULL) {
                if (strncmp(keywords[0], sensors[i].commands[j], strlen(keywords[0])) == 0) {
                    commandFound = true;
                    break;
                }
                j++;
            }
            if (commandFound) {
                break;
            }
        }
        //If we pass a command and are pulling from all available units then send them to every unit which contains the sensor and command which was found
        if (strncmp(keywords[1], "ALL", strlen("ALL")) == 0) {
            i = 0;
            int j;
            bool commandFound = false;
            while (i < NUM_OF_SENSORS) {
                while (sensors[i].commands[j] != NULL) {
                    if (strncmp(sensors[i].commands[j], keywords[0], strlen(keywords[0])) == 0) {
                        commandFound = true;
                        break;
                    }
                    j++;
                }
                if (commandFound) {
                    break;
                }
                i++;
            }
            sensor_type sensor = (sensor_type)i;
            def_message_struct msg;
            memset(&msg, 0, sizeof(msg));
            int k;
            int l;
            commandFound = false;
            for (k = 0; k < com_unit_ptr->numOfSU; k++) {
                for (l = 0; l < com_unit_ptr->SU_NUM_MODULES[k]; l++) {
                    if (com_unit_ptr->SU_AVLBL_MODULES[k][l] == sensor) {
                        msg.message[0] = '\0';
                        strncpy(msg.message, sensors[i].commands[k], MAX_MSG_LENGTH);
                        sendMessage(com_unit_ptr->SU_ADDR[i], (uint8_t*)&msg, sizeof(msg));
                        commandFound = true;
                        break;
                    }
                } 
                if (commandFound) {
                    break;
                }
            }
        } else if (keywords[1] != NULL && strncmp(keywords[1], "IND", strlen("IND")) == 0 && keywords[2] != NULL) { //Else we pass an index for a sensor unit to pull from
            int ind = 0;
            int pow = 1;
            for (i = strlen(keywords[2])-1; i >= 0; i--) {
                ind += ((int)keywords[i]-(int)'0') * pow;
                pow *= 10;
            }
            if (ind >= com_unit_ptr->numOfSU) {
                stageForReturn("Invalid index passed please try again");
            }
            def_message_struct msg;
            memset(&msg, 0, sizeof(msg));
            msg.message[0] = '\0';
            strncpy(msg.message, keywords[0], MAX_MSG_LENGTH);
            sendMessage(com_unit_ptr->SU_ADDR[ind], (uint8_t*)&msg, sizeof(msg));
        } else {
            stageForReturn("Invalid command passed from raspberry pi try again");
        }
    }
}