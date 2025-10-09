#include "sensor_units.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define MAX_QUEUE_LEN 10

/*
@breif: create a new queue which allocates enough space for the MAX_QUEUE_LEN amount of messages
*/
msg_queue::msg_queue() {
    queueHandle = xQueueCreate(MAX_QUEUE_LEN, sizeof(def_message_struct));
}
/*
@breif: sends a message to the end of the queue to be resolved
@param msg: the message to be sent
*/
bool msg_queue::send(const def_message_struct& msg) {
    if (!queueHandle) return false;
    // Use xQueueSend. It's thread-safe and interrupt-safe.
    return xQueueSend(queueHandle, &msg, portMAX_DELAY) == pdTRUE;
}

/*
@breif: write the message at the front of the queue to a buffer to be handled
@param msg: the buffer location to write into
*/
bool msg_queue::receive(def_message_struct& msg) {
    if (!queueHandle) return false;
    // Use xQueueReceive. It blocks efficiently, using zero CPU.
    return xQueueReceive(queueHandle, &msg, portMAX_DELAY) == pdTRUE;
}

/*
breif: return the queueHandle
*/
QueueHandle_t msg_queue::getQueueHandle() const {
    return queueHandle;
}
