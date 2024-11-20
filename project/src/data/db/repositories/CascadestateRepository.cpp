//
// Created by jriessner on 29.08.2022.
//

#include <sstream>
#include <utility>

#include "data/db/repositories/CascadestateRepository.h"

std::shared_ptr<CascadestateRepository> CascadestateRepository::instance;

bool CascadestateRepository::loadAll() {

    std::string sql = "SELECT * FROM cascadestate;";
    std::string retsql = databaseService->executeSqlReturn(sql);

    std::list<std::string> listCascadestates = utils::strings::splitString(retsql, ";");

    if(listCascadestates.size() > 1)
    {
        for (const auto &item : listCascadestates) {
            struct cascadestate c = getCascadestateFromQuery(item);
            allCascadestates[c.id] = c;
        }
    }

    return true;
}

struct cascadestate CascadestateRepository::getCascadestateById(int id) {
    for (const auto &cascadestate: allCascadestates)
    {
        if (cascadestate.second.id == id)
        {
            return cascadestate.second;
        }
    }

    return {};
}

struct cascadestate CascadestateRepository::getCascadestateFromQuery(const std::string &query) {
    struct cascadestate cstate;
    cstate.id = stoi(utils::strings::getValueFromQuery(query, "id", ","));
    cstate.fk_cascade = stoi(utils::strings::getValueFromQuery(query, "fk_cascade", ","));
    cstate.fk_vstate = stoi(utils::strings::getValueFromQuery(query, "fk_vesselstate", ","));
    cstate.norm_volume = stof(utils::strings::getValueFromQuery(query, "nm3", ","));

    return cstate;
}

bool CascadestateRepository::newCascadestate(struct cascadestate& cstate) {
    cstate.id = this->getNextValidId();
    std::string sqlcascadestates = "SELECT * FROM cascadestate;";
    std::string res = databaseService->executeSqlReturn(sqlcascadestates);
    std::list<std::string> listCascadestates = utils::strings::splitString(res, ";");

    if(listCascadestates.size() >= 999)
    {
        if(this->currentId >= listCascadestates.size())
        {
            this->currentId = 0;
        }

        std::stringstream deletesql;
        deletesql << "DELETE FROM cascadestate WHERE id = '" << this->currentId << "';";
        res = databaseService->executeSqlReturn(deletesql.str());
    }


    std::stringstream sql;
    sql << "insert into cascadestate ('id', 'fk_vesselstate', 'fk_cascade', 'nm3') values ('";
    sql << cstate.id << "', '";
    sql << cstate.fk_vstate << "', '";
    sql << cstate.fk_cascade << "', '";
    sql << cstate.norm_volume << "');";

    bool ret = this->databaseService->executeSql(sql.str());
    if (ret) {
        this->allCascadestates[cstate.id] = cstate;
    }

    return ret;
}

std::map<int, struct cascadestate> CascadestateRepository::getAllCascadestates() {
    return allCascadestates;
}