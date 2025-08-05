#include "sensor_units.h"


class msg_queue {
public:
    //Adds a message to the end of the queue
    void add(def_message_struct msg) {

    }
    //adds a message to the provided index
    void add(def_message_struct msg, int ind) {

    }
    //clears the queue
    void clear() {

    }
    //removes the message at index 0 index
    void pop() {

    }
    //removes the message at the given index
    void pop(int ind) {

    }
private:
    def_message_struct msgs[MAX_QUEUE_LEN];
    int finalInd;
};