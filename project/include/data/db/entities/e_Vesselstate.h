//
// Created by jriessner on 22.08.2022.
//

#ifndef H2ZMU_2_E_VESSELSTATE_H
#define H2ZMU_2_E_VESSELSTATE_H

#include <vector>
#include "e_Cascadestate.h"
#include "data/db/repositories/VesselstateRepository.h"

struct vesselstate {
    int id = -1;
    float geom_volume_0 = 0;
    float norm_volume = 0;
    float content_kg = 0;
    std::vector<int> fk_cascadestates = {};
};

#endif //H2ZMU_2_E_VESSELSTATE_H
