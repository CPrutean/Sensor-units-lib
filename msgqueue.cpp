#include "sensor_units.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define MAX_QUEUE_LEN 10

msg_queue::msg_queue() {
    queueHandle = xQueueCreate(MAX_QUEUE_LEN, sizeof(def_message_struct));
}

// Method to send a message to the queue
bool msg_queue::send(const def_message_struct& msg) {
    if (!queueHandle) return false;
    // Use xQueueSend. It's thread-safe and interrupt-safe.
    return xQueueSend(queueHandle, &msg, portMAX_DELAY) == pdTRUE;
}

// Method to receive a message from the queue
bool msg_queue::receive(def_message_struct& msg) {
    if (!queueHandle) return false;
    // Use xQueueReceive. It blocks efficiently, using zero CPU.
    return xQueueReceive(queueHandle, &msg, portMAX_DELAY) == pdTRUE;
}

// Getter for the raw handle, needed for ISRs or advanced use
QueueHandle_t msg_queue::getQueueHandle() const {
    return queueHandle;
}
