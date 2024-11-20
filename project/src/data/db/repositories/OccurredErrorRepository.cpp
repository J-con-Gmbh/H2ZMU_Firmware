//
// Created by jriessner on 24.05.2022.
//

#include "data/db/repositories/OccurredErrorRepository.h"

#include "data/db/repositories/ErrorRepository.h"
#include "Log.h"

using namespace h2zmu;

std::shared_ptr<OccurredErrorRepository> OccurredErrorRepository::instance;

struct occurrederror OccurredErrorRepository::getOccurredErrorById(int id) {
    for (const auto &occurrederror: allOccurredErrors)
    {
        if (occurrederror.second.id == id)
        {
            return occurrederror.second;
        }
    }

    return {};
}

std::map<int, struct occurrederror> OccurredErrorRepository::getAllOccurredErrors() {
    return allOccurredErrors;
}

bool OccurredErrorRepository::persistOccurredError(struct occurrederror *_occurrederror) {

    _occurrederror->id = this->currentId;
    this->currentId++;

    std::string sql = "INSERT INTO occurrederrors ( id, fk_error, occurred_timestamp, fk_hardwareinterface, fk_user) VALUES ('";
    sql += std::to_string(_occurrederror->id) + "', '";
    sql += std::to_string(_occurrederror->fk_error) + "', '";
    sql += _occurrederror->occurred_timestamp + "', '";
    sql += std::to_string(_occurrederror->occurredInterface) + "', '";
    sql += std::to_string(_occurrederror->occurredUser) + "');";

    bool ret = databaseService->executeSql(sql);
    if (ret) {
        allOccurredErrors[_occurrederror->id] = *_occurrederror;
    }

    {
        // TODO not necessary to call the cleanup every call of this function -> on startup/shutdown or with backround job (running conditions!! -> mutexing)
        if (this->allOccurredErrors.size() > 1000) {
            // delete from memory
            int overlap = (int) this->allOccurredErrors.size() - 1000;
            for (int i = 1; i <= overlap; ++i) {
                auto lastItem = this->allOccurredErrors.begin();
                this->allOccurredErrors.erase(lastItem->first);
            }

            // delete from database
            std::string sql = "DELETE FROM " + this->tableName + " WHERE id IN (SELECT id FROM " + this->tableName +
                              " ORDER BY id DESC LIMIT -1 OFFSET 1000)";
            this->databaseService->executeSql(sql);
        }
    }


    return ret;
}

std::string OccurredErrorRepository::dumpOccurredError(const occurrederror &_error) {
    std::stringstream dump;
    dump << "struct occurrederror {\n"
         << "\tid:\t\t\t" << _error.id
         << "\n\tuser:\t\t" << _error.occurredUser
         << "\n\tinterface:\t" << _error.occurredInterface
         << "\n\tresolved_timestamp:\t" << _error.resolved_timestamp
         << "\n\toccurred_timestamp:\t" << _error.occurred_timestamp
         << "\n\toccurredAt:\t" << _error.occurredAt
         << "\n}" << std::endl;

    return dump.str();
}

bool OccurredErrorRepository::loadAll() {

    this->allOccurredErrors = {};

    std::string sqloccurrederrors = "SELECT * FROM occurrederrors;";
    std::string ret = databaseService->executeSqlReturn(sqloccurrederrors);
    std::list<std::string> listOccurredErrors = utils::strings::splitString(ret, ";");

    if(listOccurredErrors.size() > 1)
    {
        for (const auto &item : listOccurredErrors) {
                struct occurrederror _error = getOccurredErrorFromQuery(item);
                this->allOccurredErrors[_error.id] = _error;
        }
    }

    return true;
}

struct occurrederror OccurredErrorRepository::getOccurredErrorFromQuery(const std::string &query) {
    struct occurrederror _error;
    _error.id = stoi(utils::strings::getValueFromQuery(query, "id", ","));
    _error.fk_error = std::stoi(utils::strings::getValueFromQuery(query, "fk_error", ","));
    _error.occurredInterface = stoi(utils::strings::getValueFromQuery(query, "fk_hardwareinterface", ","));
    _error.occurredUser = stoi(utils::strings::getValueFromQuery(query, "fk_user", ","));
    _error.occurredAt = utils::strings::getValueFromQuery(query, "errorcode", ",");
    _error.data = utils::strings::getValueFromQuery(query, "errorcode", ",");
    _error.occurred_timestamp = utils::strings::getValueFromQuery(query, "occurred_timestamp", ",");
    _error.resolved_timestamp = utils::strings::getValueFromQuery(query, "resolved_timestamp", ",");

    return _error;
}

bool OccurredErrorRepository::logError(const struct ErrorInfo& errorInfo) {
    struct occurrederror _error;

    struct error currentError = ErrorRepository::instance->getErrorByCode(errorInfo.errCode);

    int error_id = currentError.id;

    // Todo replace with currently logged in User if available
    int user_id = -1;

    _error.fk_error = error_id;
    _error.occurredInterface = errorInfo.interface;
    _error.occurredUser = user_id;
    _error.occurredAt = errorInfo.location;
    _error.data = errorInfo.data;

    Log::error({
        .message=_error.data,
        .thrownAt=_error.occurredAt,
        .loglvl=failure
    });

    return this->persistOccurredError(&_error);
}

std::vector<struct occurrederror> OccurredErrorRepository::getLastOccurredErrorsByCount(int count) {
    std::vector<struct occurrederror> ret = {};
    auto it = this->allOccurredErrors.rbegin();
    while ((it != this->allOccurredErrors.rend()) && (count > 0)) {
        ret.emplace_back(it->second);
        it++;
        count--;
    }
    std::reverse(ret.begin(), ret.end());

    return ret;
}

std::string OccurredErrorRepository::setErrorResolved(int id, std::string resolved_timestamp)
{
    struct occurrederror _error = OccurredErrorRepository::instance->getOccurredErrorById(id);
    std::string sql = "UPDATE occurrederrors SET resolved_timestamp = '";
    sql += resolved_timestamp + "' WHERE id = ";
    sql += std::to_string(id) + ";";

    this->databaseService->executeSql(sql);

    OccurredErrorRepository::instance->loadAll();

    return resolved_timestamp;
}