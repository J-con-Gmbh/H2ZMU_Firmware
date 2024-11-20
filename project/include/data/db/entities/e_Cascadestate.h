//
// Created by jriessner on 22.08.2022.
//

#ifndef H2ZMU_2_E_CASCADESTATE_H
#define H2ZMU_2_E_CASCADESTATE_H

#include <vector>
#include "e_Cascadestate.h"
#include "e_Sensorstate.h"
#include "data/db/repositories/CascadestateRepository.h"

struct cascadestate {
    int id = -1;
    int fk_cascade;
    int fk_vstate;
    float geom_volume;
    float geom_volume_corr;
    float norm_volume;
    float content_kg;
    int fk_pressure_lower_sensorstate;
    int fk_pressure_upper_sensorstate;
    std::vector<int> fk_temperature_values;
};

#endif //H2ZMU_2_E_CASCADESTATE_H
 