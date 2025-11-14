#include "tests.h"

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_all_sensors);
    RUN_TEST(test_com_unit_handling);
    RUN_TEST(test_com_unit_msg_ack);
    RUN_TEST(test_com_unit_server);
    RUN_TEST(test_msg_queue);
    RUN_TEST(test_sens_unit);
    UNITY_END();
}


void loop() {

}