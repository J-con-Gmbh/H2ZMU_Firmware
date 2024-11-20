//
// Created by jriessner on 29.08.2022.
//

#include <sstream>

#include "data/db/repositories/SensorstateRepository.h"
#include "epoch_time.h"

std::shared_ptr<SensorstateRepository> SensorstateRepository::instance;

bool SensorstateRepository::persistSensorstate(struct sensorstate& sstate) {
    sstate.id = this->getNextValidId();

    std::string sqlsensorstates = "SELECT * FROM sensorstate;";
    std::string res = databaseService->executeSqlReturn(sqlsensorstates);
    std::list<std::string> listSensorstates = utils::strings::splitString(res, ";");

    if(listSensorstates.size() >= 999)
    {
        if(this->currentId >= listSensorstates.size())
        {
            this->currentId = 0;
        }

        std::stringstream deletesql;
        deletesql << "DELETE FROM sensorstate WHERE id = '" << this->currentId << "';";
        res = databaseService->executeSqlReturn(deletesql.str());
    }

    std::stringstream sql;
    sql << "insert into sensorstate ('id', 'fk_sensor', 'fk_cstate', 'value', 'value_raw', 'timestamp') values ('";
    sql << sstate.id << "', '";
    sql << sstate.fk_sensor << "', '";
    sql << sstate.fk_cstate << "', '";
    sql << sstate.value << "', '";
    sql << sstate.value_raw << "', '";
    sql << utils::epoch_time::formatTimestamp(sstate.timestamp, "") << "');";

    bool ret = this->databaseService->executeSql(sql.str());

    if (ret) {
        this->allSensorstates[sstate.id] = sstate;
    }

    return ret;
}

bool SensorstateRepository::loadAll() {
    std::string sqlsensorstates = "SELECT * FROM sensorstate;";
    std::string ret = databaseService->executeSqlReturn(sqlsensorstates);
    std::list<std::string> listSensorstates = utils::strings::splitString(ret, ";");

    if(listSensorstates.size() > 1)
    {
        for (const auto &item : listSensorstates) {
                struct sensorstate sstate = getSensorstateFromQuery(item);
                allSensorstates[sstate.id] = sstate;
        }
    }

    return true;
}

struct sensorstate SensorstateRepository::getSensorstate(int id) {
    for (const auto &sensorstate: allSensorstates)
    {
        if (sensorstate.second.id == id)
        {
            return sensorstate.second;
        }
    }

    return {};
}

struct sensorstate SensorstateRepository::getSensorstateFromQuery(const std::string &query) {
    struct sensorstate sstate;
    sstate.id = stoi(utils::strings::getValueFromQuery(query, "id", ","));

    return sstate;
}

std::map<int, struct sensorstate> SensorstateRepository::getAllSensorstates() {
    return allSensorstates;
}