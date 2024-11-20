//
// Created by jriessner on 29.08.2022.
//

#include <sstream>

#include "data/db/repositories/VesselstateRepository.h"
#include "data/db/repositories/CascadeRepository.h"
#include "vessel/Bottle.h"
#include "data/db/repositories/BottleRepository.h"
#include "vessel/Vessel.h"

std::shared_ptr<VesselstateRepository> VesselstateRepository::instance;

bool VesselstateRepository::newVesselState(const struct vesselstate& vstate) {
    std::string sqlvesselstates = "SELECT * FROM vesselstate;";
    std::string res = databaseService->executeSqlReturn(sqlvesselstates);
    std::list<std::string> listVesselstates = utils::strings::splitString(res, ";");

    if(listVesselstates.size() >= 999)
    {
        if(this->currentId >= listVesselstates.size())
        {
            this->currentId = 0;
        }

        std::stringstream deletesql;
        deletesql << "DELETE FROM vesselstate WHERE id = '" << this->currentId << "';";
        res = databaseService->executeSqlReturn(deletesql.str());
    }

    std::string sql = "INSERT INTO vesselstate (id) values ('";
    sql += std::to_string(vstate.id);
    sql +="');";

    bool success = this->databaseService->executeSql(sql);
    if (success) {
        this->allVesselstates[vstate.id] = vstate;
    }

    return success;
}

bool VesselstateRepository::loadAll() {
    std::string sqlvesselstates = "SELECT * FROM vesselstate;";
    std::string ret = databaseService->executeSqlReturn(sqlvesselstates);
    std::list<std::string> listVesselstates = utils::strings::splitString(ret, ";");

    if(listVesselstates.size() > 1)
    {
        for (const auto &item : listVesselstates) {
                struct vesselstate _vesselstate = getVesselstateFromQuery(item);
                allVesselstates[_vesselstate.id] = _vesselstate;
        }
    }

    return true;
}

struct vesselstate VesselstateRepository::getVesselstate(int id) {
    for (const auto &vesselstate: allVesselstates)
    {
        if (vesselstate.second.id == id)
        {
            return vesselstate.second;
        }
    }

    return {};
}

bool VesselstateRepository::updateVesselState(const vesselstate &vstate) {
    if (this->allVesselstates.count(vstate.id)) {
        this->allVesselstates[vstate.id] = vstate;
        return true;
    }
    return false;
}

struct vesselstate VesselstateRepository::createVesselState() {
    struct vesselstate v_state;

    v_state.id = this->getNextValidId();

    std::map<int, cascade> cascades = CascadeRepository::instance->getAllCascades();
    std::map<int, bottle> bottles = BottleRepository::instance->getAllBottles();

    for (const auto &cascade: cascades) {
        int c_id = cascade.second.id;
        for (const auto &item: bottles) {
            if (c_id == item.second.fk_cascade) {
                struct bottle btl = item.second;
                float v_1 = Vessel::calcVolExpansion(1, btl.vol_0, btl.pressure_0, btl.vol_ref, btl.pressure_ref);
                v_state.geom_volume_0 += v_1;
            }
        }
    }

    return v_state;
}

struct vesselstate VesselstateRepository::getVesselstateFromQuery(const std::string &query) {
    struct vesselstate _vesselstate;
    _vesselstate.id = stoi(utils::strings::getValueFromQuery(query, "id", ","));

    return _vesselstate;
}

std::map<int, struct vesselstate> VesselstateRepository::getAllVesselstates() {
    return allVesselstates;
}