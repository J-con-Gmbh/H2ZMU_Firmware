//
// Created by jriessner on 29.08.2022.
//

#ifndef H2ZMU_2_SENSORSTATEREPOSITORY_H
#define H2ZMU_2_SENSORSTATEREPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/DatabaseService.h"
#include "data/db/entities/e_Sensorstate.h"

class SensorstateRepository: public Repository {
private:
    std::map<int, struct sensorstate> allSensorstates;
    struct sensorstate getSensorstateFromQuery(const std::string& query);

public:
    static std::shared_ptr<SensorstateRepository> instance;
    SensorstateRepository() : Repository("sensorstate") {}
    bool persistSensorstate(struct sensorstate& sstate);
    bool loadAll() override;
    struct sensorstate getSensorstate(int i);
    std::map<int, struct sensorstate> getAllSensorstates();
};


#endif //H2ZMU_2_SENSORSTATEREPOSITORY_H