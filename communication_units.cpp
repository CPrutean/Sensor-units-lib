#include "sensor_units.h"

const char* temp_sensor_cmds[2] = {"PULL TEMP", "PULL HUMID"};
const char* gps_sensor_cmds[2] = {"PULL LOCATION", "PULL TIME"};
const char* time_sensor_cmds[1] = {"PULL TIME"};


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

}


