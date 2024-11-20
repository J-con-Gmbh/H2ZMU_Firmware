//
// Created by jriessner on 19.06.2022.
//

#ifndef H2ZMU_2_E_HARDWAREPROTOCOL_H
#define H2ZMU_2_E_HARDWAREPROTOCOL_H

#include <string>

struct hardwareprotocol {
    int id = -1;
    std::string name;
    float rawValueMax;
    float rawValueMin;
};

#endif //H2ZMU_2_E_HARDWAREPROTOCOL_H
