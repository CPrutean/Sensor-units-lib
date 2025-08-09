#include "sensor_units.h"
//Py string word seperator
#define PY_STR_SEPER '|'

const String pyKeywordsArr[][10] = {{"PULL", "PU"}, {"TEMP AND HUMID", "GPS", "ALL"}};
const String pySensorCmds[][10] = {{"TEMP", "HUMID", "ALL"}, {"LAT AND LONG", "ALL"}};

sensor_definition sensors[NUM_OF_SENSORS+1] = {
    {temp_sensor_cmds, temp_sensor_responses, TEMP_AND_HUMID},
    {gps_sensor_cmds, gps_sensor_responses, GPS},
    {sens_unit_msgs, sens_unit_response, NUM_OF_SENSORS}
};
#define PI_SERIAL Serial


int handleMSG_CU(def_message_struct msgRecv) {
    if (com_unit_ptr == nullptr) {
        return -1;
    }
    int i = 0;
    String returnVal{""};
    returnVal += msgRecv.message;
    int i = 0;
    if (strncmp(msgRecv.message, sens_unit_response[0], MAX_CMD_LENGTH) == 0) {
        returnVal += PY_STR_SEPER;
        returnVal += status_strings[(int)msgRecv.values[0]];
    } else if (strncmp(msgRecv.message, sens_unit_response[1], MAX_CMD_LENGTH) == 0 && com_unit_ptr!=nullptr) {
        while (msgRecv.values[i]!=NULL_VALUE) {
            com_unit_ptr->SU_AVLBL_MODULES[msgRecv.channel][i] = (sensor_type)msgRecv.values[i++];
        }
    } else {
        while (i < 4 && msgRecv.values[i] != NULL_VALUE) {
            returnVal += PY_STR_SEPER;
            returnVal += floatToString(msgRecv.values[i]);
        }
    }
    stageForReturn(returnVal);
}

void stageForReturn(String str) {
    PI_SERIAL.print(str);
}


void respondPiRequest(String str) {
    String keywordArr[10];
    int len = str.length();
    int i = 0;
    int ind1 = 0;
    int ind2;
    int keyArrInd = 0;
    for (i = 0; i < len; i++) {
        if (str[i] == PY_STR_SEPER && i > 0 && i < len-1) {
            ind2 = i-1;
            keywordArr[keyArrInd] = str.substring(ind1, ind2);
            ind1 = i+1;
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
    if (keywordArr[0] == pyKeywordsArr[0][0] && keywordArr[1] == pyKeywordsArr[1][2]) {
        while (com_unit_ptr->SU_AVLBL_MODULES[i] != NULL) {
            while (com_unit_ptr->SU_AVLBL_MODULES[i][j] != NULL) {
                while (sensors[com_unit_ptr->SU_AVLBL_MODULES[i][j]].commands[k] != NULL) {
                    memset(&msg, 0, sizeof(msg));
                    strncpy(msg.message, sensors[com_unit_ptr->SU_AVLBL_MODULES[i][j]].commands[k], MAX_MSG_LENGTH);
                    sendMessage(com_unit_ptr->SU_ADDR[i], (uint8_t*)&msg, sizeof(msg));
                    k++;
                }
                j++;
            }
            i++;
        }
    } else  if (keywordArr[0] == pyKeywordsArr[0][0]) {
        i = 0;
        while (keywordArr[1] != pyKeywordsArr[1][i]) {
            i++;
        }
        j = 0;
        while (keywordArr[2] != pySensorCmds[i][j]) {
            j++;
        }

        k = 0;
        sensor_type sensor = (sensor_type)i;
        int l;
        int m;

        if (pySensorCmds[i][j] == "ALL") {
            for (l = 0; l < 6; l++) {
                if (com_unit_ptr->SU_AVLBL_MODULES[l] == NULL) {
                    break;
                }
                for (m = 0; m < 6; m++) {
                    if (com_unit_ptr->SU_AVLBL_MODULES[l][m] == NULL) {
                        break;
                    }
                    while (sensors[i].commands[k] != NULL && com_unit_ptr->SU_AVLBL_MODULES[l][m] != sensor)  {
                        memset(&msg, 0, sizeof(msg));
                        strncpy(msg.message, sensors[i].commands[k], MAX_MSG_LENGTH);
                        sendMessage(com_unit_ptr->SU_ADDR[i], (uint8_t*)&msg, sizeof(msg));
                        k++;
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
                        strncpy(msg.message, sensors[i].commands[j], MAX_MSG_LENGTH);
                        sendMessage(com_unit_ptr->SU_ADDR[l], (uint8_t*)&msg, sizeof(msg));
                    }
                }
            }
        }
    }
}


String floatToString(float value) {
    String str{""};
    if (value<0.0f) {
        str+="-";
        value = -value;
    }

    float decimals = value;
    int wholeNum = (int)value;
    decimals-=wholeNum;
    String wholePart{""};
    String decimalPart{""};
    char tempChar;

    if (wholeNum == 0) {
        wholePart+="0";
    } else {
        while (wholeNum > 0) {
            wholePart+=(char)(wholeNum%10+(int)'0');
            wholeNum/=10;
        }
        int leftInd = 0;
        int rightInd = wholePart.length()-1;
        while (leftInd<rightInd) {
            tempChar = wholePart.charAt(leftInd);
            wholePart[leftInd] = wholePart[rightInd];
            wholePart[rightInd] = tempChar;
            leftInd++;
            rightInd--;
        }
    }

    if (decimals > 0) {
        decimalPart+='.';
        int tempInt;
        int i;
        for (i = 0; i < 4; i++) {
            decimals*=10;
            int digit = (int)decimals;
            decimalPart += (char)(digit+(int)'0');
            decimals-=digit;
        }
    } else {
        decimalPart+=".0";
    }
    str+=wholePart;
    str+=decimalPart;
    return str;
}





