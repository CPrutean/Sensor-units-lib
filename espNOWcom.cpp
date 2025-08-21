#include "sensor_units.h"

static inline bool is_zero_mac(const uint8_t mac[6]) {
    for (int i = 0; i < 6; i++) {
        if (mac[i] != 0) {
            return false;
        }
    }
    return true;
}

int sendMessage(uint8_t brdcstAddr[6], uint8_t* msg, int len) {
    esp_err_t result =  esp_now_send(brdcstAddr, msg, len);
    if (result != ESP_OK) {
        return -1;
    } else {
        return 0;    
    }
}


void initCU(communication_unit *CU) {
    def_message_struct msg;
    memset(&msg, 0, sizeof(msg));
    for (int j = 0; j < CU->numOfSU; j++) {
        msg.message[0] = '\0';
        strncpy(msg.message, "PULL SENS UNITS", MAX_MSG_LENGTH - 1);
        sendMessage(CU->SU_ADDR[j], (uint8_t*)&msg, sizeof(msg));
        msg.message[0] = '\0';
        strncpy(msg.message, "PULL NAME", MAX_MSG_LENGTH-1);
        sendMessage(CU->SU_ADDR[j], (uint8_t*)&msg, sizeof(msg));
    }
}

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
