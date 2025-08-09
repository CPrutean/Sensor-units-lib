#include "sensor_units.h"


//Methods without an inherent return value will return bool to indicate succesful completions or errors in calls
class msg_queue {
public:
    msg_queue() {
        int i;
        for (i = 0; i < MAX_QUEUE_LEN; i++) {
            msgs[i].message[0] = '\0';
        }
        indOfLast = 0;
    }
    //Adds a message to the end of the queue
    bool add(def_message_struct msg) {
        int i;
        if (indOfLast == MAX_QUEUE_LEN) {
            return false;
        }
        for (i = 0; i < MAX_QUEUE_LEN; i++) {
            //If the message is an empty message or the urgency of the message at the given index is empty
            //Add them to the queue
            if (msgs[i].message[0] == '\0') {
                msgs[i] = msg;
                break;
            } else if (msgs[i].urgency<msg.urgency && msgs[i].message[0] != '\0') {
                int j;
                for (j = indOfLast; j > i; j--) {
                    msgs[j] = msgs[j-1];
                }
                msgs[i] = msg;
                break;
            }
        }
        indOfLast++;
    }
    //clears the queue
    bool clear() {
        memset(&msgs, 0, sizeof(msgs));
        for (int i = 0; i < MAX_QUEUE_LEN; i++) {
            msgs[i].message[0] = '\0';
        }
        indOfLast = 0;
    }
    //removes the message at index 0
    //No index method provided since messages should be handled in first in first out order
    //With urgency taking precedence
    bool pop() {
        int i;
        for (i = 0; i < MAX_QUEUE_LEN-1; i++) {
            if (msgs[i].message[0] == '\0') {
                break;
            }
            msgs[i] = msgs[i+1];
        }
        indOfLast--;
    }
    //Return total messages in queue
    int returnTotalMsgs() {
        return indOfLast;
    }
    
private:
    static def_message_struct msgs[MAX_QUEUE_LEN];
    static int indOfLast;
};