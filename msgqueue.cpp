#include "sensor_units.h"

msg_queue::msg_queue() : sizeOfArray(0) {
    queue_mutex = xSemaphoreCreateMutex();
    QueueHandle_t queueHandle = xQueueCreate(MAX_QUEUE_LEN, sizeof(def_message_struct)*MAX_QUEUE_LEN);
}

bool msg_queue::add(const def_message_struct msg) {
    if (!queue_mutex) return false;
    if (xSemaphoreTake(queue_mutex, portMAX_DELAY) != pdTRUE) return false;
    bool ok = false;
    if (sizeOfArray < MAX_QUEUE_LEN) {
        msgs[sizeOfArray++] = msg;
        ok = true;
    }
    xSemaphoreGive(queue_mutex);
    return ok;
}

bool msg_queue::clear() {
    if (!queue_mutex) return false;
    if (xSemaphoreTake(queue_mutex, portMAX_DELAY) != pdTRUE) return false;
    sizeOfArray = 0;
    xSemaphoreGive(queue_mutex);
    return true;
}

bool msg_queue::pop() {
    if (!queue_mutex) return false;
    if (xSemaphoreTake(queue_mutex, portMAX_DELAY) != pdTRUE) return false;
    bool ok = false;
    if (sizeOfArray > 0) {
        // shift left (small queue, acceptable; could also keep head/tail indices)
        for (size_t i = 1; i < sizeOfArray; ++i) msgs[i - 1] = msgs[i];
        --sizeOfArray;
        ok = true;
    }
    xSemaphoreGive(queue_mutex);
    return ok;
}

def_message_struct msg_queue::getFront() const {
    def_message_struct out{};
    if (!queue_mutex) return out;
    if (xSemaphoreTake(queue_mutex, portMAX_DELAY) != pdTRUE) return out;
    // const method: use non-blocking try or const_cast for locking; here we assume best effort
    if (sizeOfArray > 0) {
        out = msgs[0];
    }
    xSemaphoreGive(queue_mutex);
    return out;
}

size_t msg_queue::getSize() const {
    return sizeOfArray;
}

bool msg_queue::isEmpty() const {
    return sizeOfArray == 0;
}

QueueHandle_t msg_queue::getQueueHandle() const {
    return queueHandle;
}
