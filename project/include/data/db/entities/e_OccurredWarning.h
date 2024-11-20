//
// Created by afortino on 15.09.2023.
//

#ifndef H2ZMU_2_E_OCCURREDWARNING_H
#define H2ZMU_2_E_OCCURREDWARNING_H

#include "data/db/entities/e_Warning.h"
#include "epoch_time.h"

struct occurredwarning {
    int fk_warning; // f_key warning
    int occurredInterface; // f_key hardwareinterfaces
    int occurredUser; // f_key user
    std::string occurredAt;
    int id;
    std::string data;
    std::string occurred_timestamp = utils::epoch_time::getTimestamp("%Y-%m-%d %T.%S");
    std::string resolved_timestamp;
};

#endif //H2ZMU_2_E_OCCURREDWARNING_H
