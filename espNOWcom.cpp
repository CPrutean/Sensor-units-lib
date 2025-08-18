#include "sensor_units.h"

static inline bool is_zero_mac(const uint8_t mac[6]) {
    for (int i = 0; i < 6; i++) {
        if (mac[i] != 0) {
            return false;
        }
    }
    return true;
}

//The channel should determined within the individual .ino file for the SU to prevent intercommunication with other SU
//Each SU should be on its own channel communicating only with CU's
int init_SU_ESPNOW(sensor_unit *SU, int channel) {
    int return_val = 0;
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Failed to init espNOW");
        return_val = -1;
    }
    memcpy(SU->CU_PEER_INF.peer_addr, SU->CU_ADDR, 6);
    SU->CU_PEER_INF.encrypt = false;
    SU->CU_PEER_INF.channel = channel;

    if (esp_now_add_peer(&SU->CU_PEER_INF)!= ESP_OK) {
        Serial.println("Failed to add peer");
        return_val = -1;
    }

    esp_now_register_send_cb(def_onDataSent);
    esp_now_register_recv_cb(esp_now_recv_cb_t(def_onDataRecv));

    return return_val;
}



int init_CU_ESPNOW(communication_unit *CU) {
    #ifdef LCD_I2C_ADDR
    LCD.begin(); 
    #endif
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Failed to init espNOW");
        return -1;
    }
    int i;
    int return_val = 0;
    for (i = 0; i < CU->numOfSU; i++) {
        if (is_zero_mac(*CU->SU_ADDR)) {
            break;
        }
        memcpy(CU->SU_PEER_INF[i].peer_addr,CU->SU_ADDR[i], 6);
        CU->SU_PEER_INF[i].encrypt = false;
        CU->SU_PEER_INF[i].channel = i;

        if (esp_now_add_peer(&CU->SU_PEER_INF[i])!= ESP_OK) {
            Serial.print("Failed to add peer: ");
            Serial.println(i);
            return_val = -1;
        }
        CU->numOfSU++;
    }
    int j;
    def_message_struct msg;
    memset(&msg, 0, sizeof(msg));
    msg.message[0] = '\0';
    strncpy(msg.message, "PULL SENS UNITS", MAX_MSG_LENGTH);
    for (j = 0; j < i; j++) {
        sendMessage(CU->SU_ADDR[j], (uint8_t*)&msg, sizeof(msg));
    }
    esp_now_register_send_cb(def_onDataSent);
    esp_now_register_recv_cb(esp_now_recv_cb_t(def_onDataRecv));
    return return_val;
}



int sendMessage(uint8_t brdcstAddr[6], uint8_t* msg, int len) {
    esp_err_t result =  esp_now_send(brdcstAddr, msg, len);
    if (result != ESP_OK) {
        return -1;
    } else {
        return 0;    
    }
}

void def_onDataSent(const uint8_t *addr, esp_now_send_status_t status) {
    #ifdef DEBUG
    if (status != ESP_OK) {
        Serial.print("Message failed to send to ");
    } else {
        Serial.print("message sent to ");
    }
    Serial.println(sens_unit_ptr!=nullptr ? "SU":"CU");
    #endif
}

void def_onDataRecv(const uint8_t* adr, const uint8_t* data, int len) {
    def_message_struct msg;
    if (len == sizeof(data)) {
        memcpy(&msg, data, sizeof(msg));
        #ifdef DEBUG
        Serial.println("Message recieved");
        Serial.println(msg.message);
        #endif

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if(sens_unit_ptr == nullptr) {
            xQueueSendFromISR(com_unit_ptr->queue->getQueueHandle(), &msg, &xHigherPriorityTaskWoken);
        } else {
            xQueueSendFromISR(sens_unit_ptr->queue->getQueueHandle(), &msg, &xHigherPriorityTaskWoken);
        }
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}
