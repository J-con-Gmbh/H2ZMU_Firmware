//
// Created by jriessner on 09.06.2022.
//

#ifndef H2ZMU_2_BOTTLEREPOSITORY_H
#define H2ZMU_2_BOTTLEREPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/DatabaseService.h"
#include "data/db/entities/e_Bottle.h"

class BottleRepository: public Repository {
    std::map<int, struct bottle> allBottles;
    struct bottle getBottleFromQuery(const std::string& query);

public:
    static std::shared_ptr<BottleRepository> instance;

    BottleRepository() : Repository("bottles") {}
    bool loadAll() override;
    struct bottle getBottleById(int id);
    bool updateBottle(const struct bottle& _bottle);
    bool createBottle(struct  bottle &_bottle);
    bool deleteBottle(const struct bottle& _bottle);
    std::map<int, struct bottle> getAllBottles();
    static std::string dumpBottle(const struct bottle& _bottle);
    struct bottle setBottleSensor(int new_id, int id);
    struct bottle setBottleId(int id, int new_id);
    struct bottle setBottleCascade(int id, int cascade_id);
    struct bottle setBottleSerial(int id, std::string new_serial);
    struct bottle setBottleManufacturer(int id, std::string manufacturer);
    struct bottle setBottleBuiltyear(int id, int builtyear);
    struct bottle setBottlePressure0(int id, float pressure_0);
    struct bottle setBottleVol0(int id, float vol_0);
    struct bottle setBottlePressureRef(int id, float pressure_ref);
    struct bottle setBottleVolRef(int id, float vol_ref);
};


#endif //H2ZMU_2_BOTTLEREPOSITORY_H