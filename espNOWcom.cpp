#include "sensor_units.h"


//The channel should determined within the individual .ino file for the SU to prevent intercommunication with other SU
//Each SU should be on its own channel communicating only with CU's
void init_SU_ESPNOW(sensor_unit *SU, int channel) {
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Failed to init espNOW");
    }
    memcpy(&SU->CU_PEER_INF.peer_addr, SU->CU_ADDR, 6);
    SU->CU_PEER_INF.encrypt = true;
    SU->CU_PEER_INF.channel = channel;

    if (esp_now_add_peer(&SU->CU_PEER_INF)!= ESP_OK) {
        Serial.println("Failed to add peer");
    }

    esp_now_register_send_cb(def_onDataSent);
    esp_now_register_recv_cb(esp_now_recv_cb_t(def_onDataSent));
}

int init_CU(sensor_unit *SU_arr, int len, communication_unit *CU, char* ssid, char* pswd) {
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Failed to init espNOW");
        return -1;
    }
    int i;
    int len = sizeof(CU->available_SU)/sizeof(CU->available_SU[0]);
    int return_val = 0;
    for (i = 0; i < len; i++) {
        memcpy(&CU->SU_PEER_INF[i].peer_addr,CU->SU_ADDR[i], 6);
        CU->SU_PEER_INF[i].encrypt = true;
        CU->SU_PEER_INF[i].channel = i;

        if (esp_now_add_peer(&CU->SU_PEER_INF[i])!= ESP_OK) {
            Serial.print("Failed to add peer: ");
            Serial.println(i);
            return_val = -1;
        }

    }

    return return_val;
}

int sendMessage(uint8_t brdcstAddr[6], uint8_t* msg, int len, def_message_struct *recieve) {
    esp_err_t result =  esp_now_send(brdcstAddr, msg, len);
    if (result != ESP_OK) {
        return -1;
    } else {
        
    }
}

void def_onDataSent(const u_int8_t *addr, esp_now_send_status_t status) {
    if (status != ESP_OK) {
        Serial.print("Message failed to send to ");
    } else {
        Serial.print("message sent to ");
    }
    Serial.println(sens_unit_ptr!=nullptr ? "SU":"CU");
}

void def_onDataRecv(const u_int8_t* adr, const u_int8_t* data, int len) {
    *newMsgPtr = true;
    def_message_struct msg;
    memcpy(&msg, data, sizeof(msg));
    addToQueue(msg, queue);
}

