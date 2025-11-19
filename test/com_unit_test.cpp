#include "tests.h"

const char piBuffSeper[] = {'|', '\0'};
void test_com_unit_handling(void) {
    char buffer[1000];
    buffer[0] = '\0';

    int i;
    def_message_struct msg;
    msg.msgID = 1029231; //Random int no significance
    msg.command_ind = 0;
    msg.sensor_req = TEMP_AND_HUMID; 
    //Most values will come in the form of RESPONSE|VALUES|SENSOR_UNIT_IND|MSGID:223
    char test1[] = ""; 
    char test2[] = "";
    char test3[] = "";

}

void test_com_unit_server(void) {

}

