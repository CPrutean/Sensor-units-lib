#include "sensor_units.h"
//Methods without an inherent return value will return bool to indicate succesful completions or errors in calls
msg_queue::msg_queue() {
    msgs.register(MAX_QUEUE_LEN);
}

bool msg_queue::add(const def_message_struct msg) {
    lock_guard<std::mutex> lock(q_mutex);
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

bool msg_queue::clear() {
    std::lock_guard<std::mutex> lock(q_mutex);
    msgs.clear();
}

bool msg_queue::pop() {
    std::lock_guard<std::mutex> lock(q_mutex);
    if (msgs.empty()) {
        return false;
    }
    msgs.erase(msgs.begin());
    return true;
}

def_message_struct msg_queue::getFront() const {
    std::lock_guard<std::mutex> lock(q_mutex);
    if (msgs.empty()) {
        return def_message_struct {"MESSAGE QUEUE IS EMPTY", 0, 0};
    }
    return msgs.front();
}

size_t msg_queue::getSize() const {
    std::lock_guard<std::mutex> lock(q_mutex);
    return msgs.size();
}


bool msg_queue::isEmpty() const {
    return msgs.empty();
}
