//
// Created by afortino on 16.04.2024.
//

#ifndef H2ZMU_2_E_OCCURREDSTATUS_H
#define H2ZMU_2_E_OCCURREDSTATUS_H

#include "epoch_time.h"

struct occurredstatus {
    int id;
    uint16_t digitalIn = 0;
    uint16_t digitalOut = 0;
    std::string occurred_timestamp = utils::epoch_time::getTimestamp("%Y-%m-%d %T.%S");
};

#endif //H2ZMU_2_E_OCCURREDSTATUS_H
