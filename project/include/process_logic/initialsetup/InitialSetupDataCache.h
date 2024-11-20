//
// Created by jriessner on 07.10.23.
//

#ifndef H2ZMU_V1_INITIALSETUPDATACACHE_H
#define H2ZMU_V1_INITIALSETUPDATACACHE_H

#include <vector>
#include <map>

#include "data/db/entities/e_Bottle.h"
#include "data/db/entities/e_Cascade.h"
#include "data/db/entities/e_Sensor.h"

struct setup_data {

    bool valid = false;

    std::vector<struct sensor> sensors;
    std::vector<struct cascade> cascades;
    std::vector<struct bottle> bottles;

};

class InitialSetupDataCache {

private:
    static struct setup_data setupData;
public:

    static void add_sensor(struct sensor _sensor);

    static void add_cascade(struct cascade _cascade);
    static void add_bottle(const struct bottle& _bottle);

    static struct setup_data get_setupdata();

    static bool validate();

    static void resetDataCache();

};


#endif //H2ZMU_V1_INITIALSETUPDATACACHE_H
