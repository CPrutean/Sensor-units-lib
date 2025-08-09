#include "sensor_units.h"


//Methods without an inherent return value will return bool to indicate succesful completions or errors in calls
class msg_queue {
public:
    //Adds a message to the end of the queue
    bool add(def_message_struct msg) {

    }
    //adds a message to the provided index
    bool add(def_message_struct msg, int ind) {

    }
    //clears the queue
    bool clear() {

    }
    //removes the message at index 0 index
    bool pop() {

    }
    //removes the message at the given index
    bool pop(int ind) {

    }
    
    int returnTotalMsgs() {

    }
    
private:
    def_message_struct msgs[MAX_QUEUE_LEN];
    int finalInd;
};