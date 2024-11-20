//
// Created by jriessner on 22.08.2022.
//

#ifndef H2ZMU_2_E_SENSORSTATE_H
#define H2ZMU_2_E_SENSORSTATE_H

#include <string>

struct sensorstate {
    int id = -1;
    int fk_sensor = -1;
    int fk_cstate = -1;
    float value = 0;
    float value_raw = 0;
    time_t timestamp{};
    bool chilled = false;
    float coeff = 0;
    int seconds_back = 0;
    int count_sensor_states = 0;
};

#endif //H2ZMU_2_E_SENSORSTATE_H
