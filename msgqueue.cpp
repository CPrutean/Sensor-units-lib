#include "sensor_units.h"
//Methods without an inherent return value will return bool to indicate succesful completions or errors in calls
class msg_queue {
public:
    msg_queue() {
        msgs.reserve(MAX_QUEUE_LEN);
    }
    //Adds a message to the end of the queue
    bool add(const def_message_struct msg) {
        std::lock_guard<std::mutex> lock(q_mutex);
        if (msgs.size()>=MAX_QUEUE_LEN) {
            return false;
        }
        

        auto it = std::lower_bound(msgs.begin(), msgs.end(), msg,
            [](const def_message_struct& a, const def_message_struct& b) {
                // Custom comparator: sort by urgency in descending order.
                return a.urgency > b.urgency;
            });
        msgs.insert(it, msg);
    }

    //clears the queue
    bool clear() {
        std::lock_guard<std::mutex> lock(q_mutex);
        msgs.clear();
    }
    
    //removes the message at index 0
    //No index method provided since messages should be handled in first in first out order
    //With urgency taking precedence
    bool pop() {
        std::lock_guard<std::mutex> lock(q_mutex);
        
    }

    def_message_struct getFront() const{
        std::lock_guard<std::mutex> lock(q_mutex);
        if (msgs.empty()) {
            return def_message_struct {"MESSAGE QUEUE IS EMPTY", 0, 0};
        }
        return msgs.front();
    }

    size_t getSize() const {
        std::lock_guard<std::mutex> lock(q_mutex);
        return msgs.size();
    }
    //Return total messages in queue
    int returnTotalMsgs() const {
        std::lock_guard<std::mutex> lock(q_mutex);
        return;
    }

    bool isEmpty() const {
        return msgs.empty();
    }

private:
    std::vector<def_message_struct> msgs;
    mutable std::mutex q_mutex;
};