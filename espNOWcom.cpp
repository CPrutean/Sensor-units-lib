#include "sensor_units.h"

/*
@breif: checks if the mac address is all 0 to determine if its a valid or invalid address
@param mac[6]: the mac address passed
*/
static inline bool is_zero_mac(const uint8_t mac[6]) {
    for (int i = 0; i < 6; i++) {
        if (mac[i] != 0) {
            return false;
        }
    }
    return true;
}


/*
@breif: sends a message to an esp32 device with the provided mac address
@param brdcstAddr[6]: the broadcast address provided
@param *msg: the data to be sent
@param len: the amount of bytes being sent
@return: returns -1 if it fails to send and 0 if it was succesful
*/
int sendMessage(uint8_t brdcstAddr[6], uint8_t* msg, int len) {
    esp_err_t result =  esp_now_send(brdcstAddr, msg, len);
    if (result != ESP_OK) {
        return -1;
    } else {
        return 0;    
    }
}

/*
@breif: initializes the sensor units available and the status of the sensor units in the provided broadcast addresses
@param *CU: the communication unit being initialized
*/
void initCU(communication_unit *CU) {
    int i;
    int j;
    //Assume non functional state with 0 sensors as default
    for (i = 0; i < 6; i++) {
        for (j = 0; j < NUM_OF_SENSORS-1; j++) {
            CU->SU_AVLBL_MODULES[i][j] = NUM_OF_SENSORS;
        }
        CU->SU_NUM_MODULES[i] = 0;
        CU->status[i] = OFFLINE;
    }

    def_message_struct msg;
    memset(&msg, 0, sizeof(msg));
    for (int j = 0; j < CU->numOfSU; j++) {
        msg.message[0] = '\0';
        strncpy(msg.message, sens_unit_msgs[0], MAX_MSG_LENGTH - 1);
        sendMessage(CU->SU_ADDR[j], (uint8_t*)&msg, sizeof(msg));
        msg.message[0] = '\0';
        strncpy(msg.message, sens_unit_msgs[1], MAX_MSG_LENGTH-1);
        sendMessage(CU->SU_ADDR[j], (uint8_t*)&msg, sizeof(msg));
        msg.message[0] = '\0';
        strncpy(msg.message, sens_unit_msgs[2], MAX_MSG_LENGTH-1);
        sendMessage(CU->SU_ADDR[j], (uint8_t*)&msg, sizeof(msg));
    }
}

/*
@breif: initializes the sensor unit with the name stored in EEPROM if one exits otherwise initialize a new name
@param: *SU the sensor unit being intialized
@return: returns if the device contained a name in EEPROM memory or not
*/
bool initSU(sensor_unit *SU) {
    def_message_struct temp;
    if (!readFromEEPROM(SU->name, MAX_NAME_LEN, &temp)) {
        #ifdef DEBUG
        Serial.print("Failed to read name from eeprom error code");
        Serial.println(temp.message);
        #endif
        return false;
    } else {
        return true;
    }
}
