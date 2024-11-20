//
// Created by jriessner on 24.05.2022.
//

#ifndef H2ZMU_2_E_OCCURREDERROR_H
#define H2ZMU_2_E_OCCURREDERROR_H

#include "data/db/entities/e_Error.h"
#include "epoch_time.h"

struct occurrederror {
    int fk_error; // f_key error
    int occurredInterface; // f_key hardwareinterfaces
    int occurredUser; // f_key user
    std::string occurredAt;
    int id;
    std::string data;
    std::string occurred_timestamp = utils::epoch_time::getTimestamp("%Y-%m-%d %T.%S");
    std::string resolved_timestamp;
};

#endif //H2ZMU_2_E_OCCURREDERROR_H
