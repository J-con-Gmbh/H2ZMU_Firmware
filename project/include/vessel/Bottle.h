//
// Created by jriessner on 10.06.2022.
//

#ifndef H2ZMU_2_BOTTLE_H
#define H2ZMU_2_BOTTLE_H

#include <memory>

#include "data/db/entities/e_Bottle.h"
#include "TempSensor.h"

class Bottle {
public:
    std::shared_ptr<TempSensor> sensor = nullptr;
    bottle btl;
};


#endif //H2ZMU_2_BOTTLE_H
