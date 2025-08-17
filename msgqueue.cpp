#include "sensor_units.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define MAX_QUEUE_LEN 10

class msg_queue {
private:
    QueueHandle_t queueHandle;

public:
    msg_queue() {
        queueHandle = xQueueCreate(MAX_QUEUE_LEN, sizeof(def_message_struct));
    }

    // Method to send a message to the queue
    bool send(const def_message_struct& msg) {
        if (!queueHandle) return false;
        // Use xQueueSend. It's thread-safe and interrupt-safe.
        return xQueueSend(queueHandle, &msg, portMAX_DELAY) == pdTRUE;
    }

    // Method to receive a message from the queue
    bool receive(def_message_struct& msg) {
        if (!queueHandle) return false;
        // Use xQueueReceive. It blocks efficiently, using zero CPU.
        return xQueueReceive(queueHandle, &msg, portMAX_DELAY) == pdTRUE;
    }

    // Getter for the raw handle, needed for ISRs or advanced use
    QueueHandle_t getQueueHandle() const {
        return queueHandle;
    }
};