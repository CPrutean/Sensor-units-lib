#include "tests.h"

const char piBuffSeper[] = {'|', '\0'};
void test_com_unit_handling(void) {
    char buffer[1000];
    buffer[0] = '\0';

    int i;
    def_message_struct msg;
    msg.msgID = 1029231; //Random int no significance
    int j;
    double defaultVal = 20.3;
    char defaultStr[] = "temp"; //Default string value for string tests.

    for (i = 0; i < NUM_OF_SENSORS; i++) {
        msg.sensor_req = static_cast<sensor_type>(i);
        j = 0;
        while (sensors[i].responses[j] != NULL) {
            snprintf(buffer, sizeof(buffer)-1, "%s", sensors[i].responses[j]);
            if (sensors[i].values[j] == DOUBLE_T) {
                snprintf(buffer, sizeof(buffer)-strlen(buffer)-1, "%d", defaultVal);
            } else {
                snprintf(buffer, sizeof(buffer)-strlen(buffer), "%s", defaultStr);            
            }
            msg.command_ind = j;
            snprintf(buffer, sizeof(buffer)-strlen(buffer)-1, "%ld", msg.msgID);

            j++;
        }


    }
}

void test_com_unit_server(void) {

}

