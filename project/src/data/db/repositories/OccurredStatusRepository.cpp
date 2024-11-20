//
// Created by afortino on 16.04.2024.
//

#include "data/db/repositories/OccurredStatusRepository.h"

#include "Log.h"

using namespace h2zmu;

std::shared_ptr<OccurredStatusRepository> OccurredStatusRepository::instance;

struct occurredstatus OccurredStatusRepository::getOccurredStatusById(int id) {
    for (const auto &occurredstatus: allOccurredStatus)
    {
        if (occurredstatus.second.id == id)
        {
            return occurredstatus.second;
        }
    }

    return {};
}

std::map<int, struct occurredstatus> OccurredStatusRepository::getAllOccurredStatus() {
    return allOccurredStatus;
}

bool OccurredStatusRepository::persistOccurredStatus(struct occurredstatus *_occurredstatus) {

    _occurredstatus->id = this->currentId;
    this->currentId++;

    std::string sql = "INSERT INTO occurredstatus ( id, digitalIn, ,digitalOut, occurred_timestamp) VALUES ('";
    sql += std::to_string(_occurredstatus->id) + "', '";
    sql += std::to_string(_occurredstatus->digitalIn) + "', '";
    sql += std::to_string(_occurredstatus->digitalOut) + "', '";
    sql += _occurredstatus->occurred_timestamp + "');";

    bool ret = databaseService->executeSql(sql);
    if (ret) {
        allOccurredStatus[_occurredstatus->id] = *_occurredstatus;
    }

    {
        // TODO not necessary to call the cleanup every call of this function -> on startup/shutdown or with backround job (running conditions!! -> mutexing)
        if (this->allOccurredStatus.size() > 1000) {
            // delete from memory
            int overlap = (int) this->allOccurredStatus.size() - 1000;
            for (int i = 1; i <= overlap; ++i) {
                auto lastItem = this->allOccurredStatus.begin();
                this->allOccurredStatus.erase(lastItem->first);
            }

            // delete from database
            std::string sql = "DELETE FROM " + this->tableName + " WHERE id IN (SELECT id FROM " + this->tableName +
                              " ORDER BY id DESC LIMIT -1 OFFSET 1000)";
            this->databaseService->executeSql(sql);
        }
    }


    return ret;
}

std::string OccurredStatusRepository::dumpOccurredStatus(const occurredstatus &_status) {
    std::stringstream dump;
    dump << "struct occurredstatus {\n"
        << "\tid:\t\t\t" << _status.id
        << "\n\tdigitalIn:\t" << _status.digitalIn 
        << "\n\tdigitalOut:\t" << _status.digitalOut
        << "\n\toccurred_timestamp:\t" << _status.occurred_timestamp
        << "\n}" << std::endl;

    return dump.str();
}

bool OccurredStatusRepository::loadAll() {

    this->allOccurredStatus = {};

    std::string sqloccurredstatus = "SELECT * FROM occurredstatus;";
    std::string ret = databaseService->executeSqlReturn(sqloccurredstatus);
    std::list<std::string> listOccurredStatus = utils::strings::splitString(ret, ";");

    if(listOccurredStatus.size() > 1)
    {
        for (const auto &item : listOccurredStatus) {
                struct occurredstatus _status = getOccurredStatusFromQuery(item);
                this->allOccurredStatus[_status.id] = _status;
        }
    }

    return true;
}

struct occurredstatus OccurredStatusRepository::getOccurredStatusFromQuery(const std::string &query) {
    struct occurredstatus _status;
    _status.id = stoi(utils::strings::getValueFromQuery(query, "id", ","));
    _status.digitalIn = std::stoi(utils::strings::getValueFromQuery(query, "digitalIn", ","));
    _status.digitalOut = std::stoi(utils::strings::getValueFromQuery(query, "digitalOut", ","));
    _status.occurred_timestamp = utils::strings::getValueFromQuery(query, "occurred_timestamp", ",");

    return _status;
}

std::vector<struct occurredstatus> OccurredStatusRepository::getLastOccurredStatusByCount(int count) {
    std::vector<struct occurredstatus> ret = {};
    auto it = this->allOccurredStatus.rbegin();
    while ((it != this->allOccurredStatus.rend()) && (count > 0)) {
        ret.emplace_back(it->second);
        it++;
        count--;
    }
    std::reverse(ret.begin(), ret.end());

    return ret;
}
