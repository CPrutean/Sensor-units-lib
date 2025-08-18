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

void onDataSent(const uint8_t *addr, esp_now_send_status_t status) {
    Serial.println("PACKET SENT");
    #ifdef DEBUG
    if (status != ESP_OK) {
        Serial.print("Message failed to send to ");
    } else {
        Serial.print("message sent to ");
    }
    Serial.println(sens_unit_ptr!=nullptr ? "SU":"CU");
    #endif
}

void onDataRecv(const uint8_t* adr, const uint8_t* data, int len) {
    Serial.println("Message recieved");
    if (len == sizeof(def_message_struct)) {
        def_message_struct msg;
        memcpy(&msg, data, sizeof(msg));
        
        #ifdef DEBUG
        Serial.println("Message received");
        Serial.println(msg.message);
        #endif

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (sens_unit_ptr == nullptr) {
            int i;
            int j;
            bool channelFound = true;
            for (i = 0; i < com_unit_ptr->numOfSU; i++) {
                for (j = 0; j = 6; j++) {
                    if (com_unit_ptr->SU_ADDR[i][j] != adr[j]) {
                        channelFound = false;
                        break;
                    }
                }
                if (channelFound) {
                    msg.suInd = j;
                    break;
                }
            }
            xQueueSendFromISR(com_unit_ptr->queue->getQueueHandle(), &msg, &xHigherPriorityTaskWoken);
        } else {
            xQueueSendFromISR(sens_unit_ptr->queue->getQueueHandle(), &msg, &xHigherPriorityTaskWoken);
        }
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    } else {
        #ifdef DEBUG
        Serial.printf("Received message of wrong size. Got: %d, Expected: %d\n", len, sizeof(def_message_struct));
        #endif
    }
}


//The channel should determined within the individual .ino file for the SU to prevent intercommunication with other SU
//Each SU should be on its own channel communicating only with CU's
int init_SU_ESPNOW(sensor_unit *SU) {
    int return_val = 0;
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    if (esp_now_init() != ESP_OK) {
        #ifdef DEBUG
        Serial.println("Failed to init espNOW");
        #endif
        return_val = -1;
    } else {
        #ifdef DEBUG
        Serial.println("Succesfully initialized ESPNOW");
        #endif
    }
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, SU->CU_ADDR, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo)!= ESP_OK) {
        #ifdef DEBUG
        Serial.println("Failed to add peer");
        #endif
        return_val = -1;
    } else {
        #ifdef DEBUG
        Serial.println("Succesfully addded peer");
        #endif
    }
    #ifdef DEBUG
    Serial.println("Registered callbacks");
    #endif
    if (esp_now_register_send_cb(onDataSent) != ESP_OK) {
        #ifdef DEBUG
        Serial.println("Failed to register sent callback");
        #endif
    } else {
        #ifdef DEBUG
        Serial.println("Succesfully initialized send callback");
        #endif
    }
    if (esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv)) != ESP_OK) {
        #ifdef DEBUG
        Serial.println("Failed to register recieve callback");
        #endif
    } else {
        #ifdef DEBUG
        Serial.println("Succesfully initialized recieve callback");
        #endif
    }
    return return_val;
}



int init_CU_ESPNOW(communication_unit *CU) {
    #ifdef LCD_I2C_ADDR
    LCD.begin(); 
    #endif
    // ... (LCD init if needed) ...
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    if (esp_now_init() != ESP_OK) {
        #ifdef DEBUG
        Serial.println("Failed to init espNOW");
        #endif
        return -1;
    }

    int return_val = 0;
    int registered_peers = 0; // Use a local counter
    esp_now_peer_info_t peerInfo[CU->numOfSU];
    for (int i = 0; i < CU->numOfSU; i++) { // Assuming `maxNumOfSU` is the size of the array
        if (is_zero_mac(CU->SU_ADDR[i])) {
            break; // Stop if we find an empty MAC address
        }
        memcpy(peerInfo[i].peer_addr, CU->SU_ADDR[i], 6);
        peerInfo[i].channel = 0;
        peerInfo[i].encrypt = 0;
        if (esp_now_add_peer(&peerInfo[i]) != ESP_OK) {
            #ifdef DEBUG
            Serial.print("Failed to add peer: ");
            Serial.println(i);
            #endif
            return_val = -1;
        } else {
            #ifdef DEBUG
            Serial.println("Succesfully added peer");
            #endif
            registered_peers++;
        }
    }
    
    CU->numOfSU = registered_peers;
    #ifdef DEBUG
    Serial.println("Registered callbacks");
    #endif    

    if (esp_now_register_send_cb(onDataSent) != ESP_OK) {
        #ifdef DEBUG
        Serial.println("Failed to register sent callback");
        #endif
    } else {
        #ifdef DEBUG
        Serial.println("Succesfully initialized send callback");
        #endif
    }
    if (esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv)) != ESP_OK) {
        #ifdef DEBUG
        Serial.println("Failed to register recieve callback");
        #endif
    } else {
        #ifdef DEBUG
        Serial.println("Succesfully initialized recieve callback");
        #endif
    }
 
    return return_val;
}


void initCU(communication_unit *CU) {
    def_message_struct msg;
    memset(&msg, 0, sizeof(msg));
    strncpy(msg.message, "PULL SENS UNITS", MAX_MSG_LENGTH - 1);

    for (int j = 0; j < CU->numOfSU; j++) {
        sendMessage(CU->SU_ADDR[j], (uint8_t*)&msg, sizeof(msg));
    }
}

