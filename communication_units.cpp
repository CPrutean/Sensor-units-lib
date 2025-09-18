#include "sensor_units.h"

//Py string word seperator
char pyStrSeper[] = {'|', '\0'};
char piBufferSeper[] {'\n', '\0'};



void stageForReturn(char* str) {
    char tempStr[MAX_MSG_LENGTH];
    tempStr[0] = '\0';
    strncpy(tempStr, str, sizeof(tempStr));
    strncat(tempStr, piBufferSeper, sizeof(tempStr)-strlen(tempStr));
    Serial.print(tempStr);
}

/*
@brief: basic substring function to write a start of a function
@param source: the source string which we are taking from
@param start: where we start copying from
@param len: the length of how much we are copying
@param dest: the destination for where we are copying into
@param bufferLen: the length of the buffer which we are copying into
@return: returns -1 if it failed to copy into the buffer and 0 if it succesfully copied into the buffer
*/
int substring(const char* source, int start, int len, char* dest, int bufferLen) {
    if (dest == NULL || len > bufferLen) {
        return -1;
    }
    strncpy(dest, source+start, len);
    dest[len] = '\0';
    return 0;
}

const int MAXPYSTRINGLEN = 1000;

/*
@brief: handles the messages recieved from sensor units to return back to the raspberry pi
@param msgRecv: the message recieved from the sensor unit
@param SUInd: the index of the SU we recieved it from
@return: returns -1 if it failed to execute and 0 if it executed with no issues
*/
int handleMSG_CU(const def_message_struct& msgRecv) {
    if (com_unit_ptr == nullptr) {
        #ifdef DEBUG
        Serial.print("Com_unit_ptr was never initialized");
        #endif
        return -1;
    }
    char returnVal[MAXPYSTRINGLEN];
    int SUInd;
    int i;
    int j;
    bool macFound = true;
    for (i = 0; i < com_unit_ptr->numOfSU; i++) {
        for (j = 0; j < 6; j++) {
            if (com_unit_ptr->SU_ADDR[i][j] != msgRecv.senderMac[j]) {
                macFound = false;
                break;
            }
        }
        if (macFound) {
            SUInd = i;
            break;
        } else {
            macFound = true;
        }
    }
    snprintf(returnVal, sizeof(returnVal), "%s", msgRecv.message);

    if (strncmp(msgRecv.message, sens_unit_response[0], strlen(sens_unit_response[0])) == 0) {
        strncat(returnVal, pyStrSeper, sizeof(returnVal) - strlen(returnVal) - 1);

        int status_index = (int)msgRecv.values[0];
        if (status_index >= 0 && status_index < 3) {
            strncat(returnVal, status_strings[status_index], sizeof(returnVal) - strlen(returnVal) - 1);
        } else {
            strncat(returnVal, "INVALID_STATUS", sizeof(returnVal) - strlen(returnVal) - 1);
        }
    } else if (strncmp(msgRecv.message, sens_unit_response[1], strlen(sens_unit_response[1])) == 0) {
        for (int i = 0; i < msgRecv.numValues; i++) {
            com_unit_ptr->SU_AVLBL_MODULES[SUInd][i] = (sensor_type)msgRecv.values[i];
            com_unit_ptr->SU_NUM_MODULES[SUInd]++;
            strncat(returnVal, pyStrSeper, sizeof(returnVal)-strlen(returnVal)-1);
            strncat(returnVal, sens_unit_strings[(int)msgRecv.values[i]], sizeof(returnVal)-strlen(returnVal)-1);
        }
    } else {
        for (int i = 0; i < msgRecv.numValues; i++) {
            strncat(returnVal, pyStrSeper, sizeof(returnVal) - strlen(returnVal) - 1);
            int len = snprintf(NULL, 0, "%f", msgRecv.values[i]);
            char tempStr[30];
            snprintf(tempStr, len + 1, "%f", msgRecv.values[i]);
            strncat(returnVal, tempStr, sizeof(returnVal) - strlen(returnVal) - 1);
        }
        if (msgRecv.value[0] != '\0') {
            strncat(returnVal, msgRecv.value, sizeof(returnVal)-strlen(returnVal)-1);
            strncat(returnVal, pyStrSeper, sizeof(returnVal)-strlen(returnVal)-1);
        }
    }
    char SUStr[2];
    SUStr[0] = (char)((int)'0'+SUInd);
    SUStr[1] = '\0';
    strncat(returnVal, pyStrSeper, sizeof(returnVal)-strlen(returnVal)-1);
    strncat(returnVal, SUStr, sizeof(returnVal)-strlen(returnVal)-1);
    stageForReturn(returnVal);
    return 0; 
}


const int maxKeywordLen = 30;


/*
@breif: responsed to the commands sent by the raspberry pi
@param str: the string passed by the raspberry pi
*/
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
            #ifdef DEBUG
            stageForReturn(keywords[keyArrInd]);
            #endif
            keyArrInd++;
            lastInd = i+1;
        }
    }
    //Provide the raspberry pi with all of the available sensor units
    if (keywords[0] != NULL && keywords[1] != NULL && strncmp(keywords[0], "INIT", strlen("INIT")) == 0 && strncmp(keywords[1], "PI", strlen("PI")) == 0) {
        int i;
        int j;
        char initStr[MAXPYSTRINGLEN];
        initStr[0] = '\0';
        int strLength = 0;
        //When defining a sensor we pass the header 'SENSOR', the pyStringSeper and all of the commands and responses to the commands are seperated by commas
        //To allow raspberry pi to sort the values
        //Excluding '
        //The order goes SENSOR|NAME|'commands,seperated,by,commas,'|'return,strings,seperated,by,commas,|
        for (i = 0; i < NUM_OF_SENSORS; i++) {
            strLength = snprintf(initStr, sizeof(initStr), "%s", "SENSOR");
            //minus 2 in these cases to also include the comma that seperates them
            strncat(initStr, pyStrSeper, sizeof(initStr)-strlen(initStr)-2);
            strncat(initStr, sensors[i].name, sizeof(initStr)-strlen(initStr)-1);
            strncat(initStr, pyStrSeper, sizeof(initStr)-strlen(initStr)-1);
            while (sensors[i].commands[j] != NULL) {
                if (strlen(sensors[i].commands[j]) < sizeof(initStr)-strlen(initStr)-1) {
                    strncat(initStr, sensors[i].commands[j], sizeof(initStr)-strlen(initStr)-1);
                    strncat(initStr, ",", sizeof(initStr)-strlen(initStr)-1);
                } else {
                    stageForReturn("Increase string buffer size in INIT|PI functionality");
                    return;
                }
                j++;
            }
            j = 0;
            strncat(initStr, pyStrSeper, sizeof(initStr)-strlen(initStr)-1);
            while (sensors[i].responses[j] != NULL) {
                if (strlen(sensors[i].responses[j]) < sizeof(initStr)-strlen(initStr)-1) {
                    strncat(initStr, sensors[i].responses[j], sizeof(initStr)-strlen(initStr)-1);
                    strncat(initStr, ",", sizeof(initStr)-strlen(initStr)-1);
                } else {
                    stageForReturn("Increase string buffer size in INIT|PI functionality");
                    return;
                }
                j++;
            }
            j = 0;
            if (sizeof(initStr)-strlen(initStr)-1 > 1) {
                strncat(initStr, pyStrSeper, sizeof(initStr)-strlen(initStr)-1);
                stageForReturn(initStr);
            } else {
                stageForReturn("Increase string buffer size in INIT|PI functionality");
                return;
            }
            //Reset string
            strLength = 0;
            initStr[0] = '\0';
        }
        char numOfSU[20];
        numOfSU[0] = '\0';
        strncpy(numOfSU, "NUM_OF_SU|", sizeof(numOfSU));
        char ind[2];
        ind[0] = (char)(com_unit_ptr->numOfSU+(int)'0');
        ind[1] = '\0';
        strncat(numOfSU, ind, sizeof(numOfSU)-strlen(numOfSU)-1);
        stageForReturn(numOfSU);
        //This will then send back the sensor units, their names and their status
        initCU(com_unit_ptr);
        return;
    }
    //Case for pulling all available data from all available sensor unitsz
    if (keywords[0] != NULL && keywords[1] != NULL && strncmp(keywords[0], "PULL", strlen(keywords[0])) == 0 && strncmp(keywords[1], "ALL", strlen("ALL")) == 0) {
        int j;
        int k;
        int l = 0;
        def_message_struct msg;
        memset(&msg, 0, sizeof(msg));
        for (i = 0; i < com_unit_ptr->numOfSU; i++) {
            //Skip the push name command
            j = 0;
            while (sens_unit_msgs[j] != NULL) {
                if (strncmp(sensors[BASE_SENS_UNIT].commands[j], "PULL", strlen("PULL")) == 0) {
                    msg.message[0] = '\0';
                    msg.strlen = snprintf(msg.message, MAX_MSG_LENGTH, "%s", sensors[BASE_SENS_UNIT].commands[j]);
                }
                j++;
            }
            for (k = 0; k < com_unit_ptr->SU_NUM_MODULES[i]; k++) {
                switch(com_unit_ptr->SU_AVLBL_MODULES[i][k]) {
                    case (TEMP_AND_HUMID):
                        while (sensors[TEMP_AND_HUMID].commands[l] != NULL) {
                            if (strncmp(sensors[TEMP_AND_HUMID].commands[l], "PULL", strlen("PULL")) == 0) {
                                msg.message[0] = '\0';
                                strncpy(msg.message, sensors[TEMP_AND_HUMID].commands[l], MAX_MSG_LENGTH);
                                sendMessage(com_unit_ptr->SU_ADDR[i], (uint8_t*)&msg, sizeof(msg));
                            }
                            l++;
                        }
                        break;
                    case (GPS):
                        while (sensors[GPS].commands[l] != NULL) {
                            if (strncmp(sensors[TEMP_AND_HUMID].commands[l], "PULL", strlen("PULL")) == 0) {
                                msg.message[0] = '\0';
                                strncpy(msg.message, sensors[GPS].commands[l], MAX_MSG_LENGTH);
                                sendMessage(com_unit_ptr->SU_ADDR[i], (uint8_t*)&msg, sizeof(msg));
                            }
                            l++;
                        }
                        break;
                    default:
                        break;
                }
                l = 0;
            }
        }
    } else {
        int i;
        int j = 0;
        bool sensor_found = false;
        for (i = 0; i < NUM_OF_SENSORS; i++) {
            j = 0;
            while (sensors[i].commands[j] != NULL) {
                if (strncmp(sensors[i].commands[j], keywords[0], sizeof(sensors[i].commands[j])) == 0) {
                    sensor_found = true;
                }
                j++;
            }
            if (sensor_found) {
                break;
            }
        }
        if (!sensor_found) {
            stageForReturn("Sensor wasnt found for this command");
            return;
        }
        
        sensor_type sensor = (sensor_type)i;
        //If we pass a command and are pulling from all available units then send them to every unit which contains the sensor and command which was found
        if (strncmp(keywords[1], "ALL", strlen("ALL")) == 0) {
            i = 0;
            int j;

            def_message_struct msg;
            memset(&msg, 0, sizeof(msg));
            bool commandFound = false;
            for (i = 0; i < com_unit_ptr->numOfSU; i++) {
                for (j = 0; j < com_unit_ptr->SU_NUM_MODULES[i]; i++) {
                    if (sensor == com_unit_ptr->SU_AVLBL_MODULES[i][j]) {
                        msg.message[0] = '\0';
                        msg.strlen = snprintf(msg.message, sizeof(msg.message), "%s", keywords[0]);
                        sendMessage(com_unit_ptr->SU_ADDR[i], (uint8_t*)&msg.message, sizeof(msg));
                    }
                }
            }
            
        } else if (keywords[1] != NULL && strncmp(keywords[1], "IND", strlen("IND")) == 0 && keywords[2] != NULL) { //Else we pass an index for a sensor unit to pull from
            int ind = 0;
            int pow = 1;
            for (i = strlen(keywords[2])-1; i >= 0; i--) {
                ind += ((int)keywords[2][i]-(int)'0') * pow;
                pow *= 10;
            }
            if (ind > com_unit_ptr->numOfSU) {
                stageForReturn("Invalid index passed please try again");
            }
            j = 0;            
            bool hasSens = false;
            for (i = 0; i < com_unit_ptr->SU_NUM_MODULES[ind]; i++) {
                if (com_unit_ptr->SU_AVLBL_MODULES[ind][i] == sensor) {
                    hasSens = true;
                    break;
                }
            }
            if (hasSens) {
                def_message_struct msg;
                memset(&msg, 0, sizeof(msg));
                msg.message[0] = '\0';
                strncpy(msg.message, keywords[0], MAX_MSG_LENGTH);
                sendMessage(com_unit_ptr->SU_ADDR[ind], (uint8_t*)&msg, sizeof(msg));
            } else {
                stageForReturn("Indicator doesnt have that sensor try again");
            }   
        } else {
            stageForReturn("Invalid command passed from raspberry pi try again");
        }
    }
}