//
// Created by jriessner on 22.08.2022.
//

#ifndef H2ZMU_2_E_MEASUREMENT_H
#define H2ZMU_2_E_MEASUREMENT_H

#include <ctime>
#include <list>

#include "e_OccurredError.h"
#include "e_OccurredWarning.h"
#include "e_Vesselstate.h"

struct measurement {
    int id = -1;
    int user_id = -1;
    float amountStartNm3 = -1;
    float amountEndNm3 = -1;
    time_t timestamp_start;
    time_t timestamp_end;
    std::string external_measurement_id;
    int fk_vessel_state_start = -1;
    int fk_vessel_state_end = -1;
    std::list<int> fk_thrown_errors;
    std::list<int> fk_thrown_warnings;
    bool valid = false;
    bool finished = false;
};

#endif //H2ZMU_2_E_MEASUREMENT_H
