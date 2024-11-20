//
// Created by jriessner on 03.07.23.
//

#ifndef UTILS_ITEM_H
#define UTILS_ITEM_H

#include <ctime>

#include "../epoch_time.h"

template <typename T>
class Item {
    T payload;
    time_t createdAt{};

public:
    Item(T payload) {
        this->payload = payload;
        this->createdAt = utils::epoch_time::getUnixTimestamp();
    }

    time_t getCreatedAt() {
        return this->createdAt;
    }

    T getPayload() {
        return this->payload;
    }

};



#endif //UTILS_ITEM_H
