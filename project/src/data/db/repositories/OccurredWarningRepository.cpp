//
// Created by jriessner on 24.05.2022.
//

#include "data/db/repositories/OccurredWarningRepository.h"
#include "Log.h"

using namespace h2zmu;

std::shared_ptr<OccurredWarningRepository> OccurredWarningRepository::instance;

struct occurredwarning OccurredWarningRepository::getOccurredWarningById(int id) {
    return allOccurredWarnings[id];
}

std::map<int, struct occurredwarning> OccurredWarningRepository::getAllOccurredWarnings() {
    return allOccurredWarnings;
}



bool OccurredWarningRepository::persistOccurredWarning(struct occurredwarning *_occurredwarning) {

    // TODO Unnötige Datenbankzugriffe vermeiden
    //  -> Warnings werden initial geladen und können aus der member variable "this->allOccurredWarnings" geholt werden

    std::string sqloccurredwarnings = "SELECT * FROM occurredwarnings;";
    std::string res = databaseService->executeSqlReturn(sqloccurredwarnings);
    std::list<std::string> listOccurredWarnings = utils::strings::splitString(res, ";");

    if(listOccurredWarnings.size() >= 1000)
    {
        sqloccurredwarnings = "SELECT * FROM occurredwarnings WHERE occurred_timestamp = (SELECT MIN(occurred_timestamp) FROM occurredwarnings);";
        res = databaseService->executeSqlReturn(sqloccurredwarnings);
        listOccurredWarnings = utils::strings::splitString(res, ";");
        int oldest_timeoccurred_id = 0; 

        for (const auto &item : listOccurredWarnings) {
                struct occurredwarning _warning = getOccurredWarningFromQuery(item);
                oldest_timeoccurred_id = _warning.id;
        }
        
        std::stringstream deletesql;
        deletesql << "DELETE FROM occurredwarnings WHERE id = '" << oldest_timeoccurred_id << "';";
        res = databaseService->executeSqlReturn(deletesql.str());
        this->currentId = oldest_timeoccurred_id;
    }

    _occurredwarning->id = this->currentId;
    this->currentId++;

    std::string sql = "INSERT INTO occurredwarnings ( id, fk_warning, occurred_timestamp, fk_hardwareinterface, fk_user) VALUES ('";
    sql += std::to_string(_occurredwarning->id) + "', '";
    sql += std::to_string(_occurredwarning->fk_warning) + "', '";
    sql += _occurredwarning->occurred_timestamp + "', '";
    sql += std::to_string(_occurredwarning->occurredInterface) + "', '";
    sql += std::to_string(_occurredwarning->occurredUser) + "');";

    bool ret = databaseService->executeSql(sql);
    if (ret) {
        allOccurredWarnings[_occurredwarning->id] = *_occurredwarning;
    }
    return ret;
}

std::string OccurredWarningRepository::dumpOccurredWarning(const occurredwarning &_warning) {
    std::stringstream dump;
    dump << "struct occurredwarning {\n"
         << "\tid:\t\t\t" << _warning.id
         << "\n\tuser:\t\t" << _warning.occurredUser
         << "\n\tinterface:\t" << _warning.occurredInterface
         << "\n\toccurred_timestamp:\t" << _warning.occurred_timestamp
         << "\n\toccurredAt:\t" << _warning.occurredAt
         << "\n}" << std::endl;

    return dump.str();
}

bool OccurredWarningRepository::loadAll() {
    std::string sqloccurredwarnings = "SELECT * FROM occurredwarnings;";
    std::string ret = databaseService->executeSqlReturn(sqloccurredwarnings);
    std::list<std::string> listOccurredWarnings = utils::strings::splitString(ret, ";");

    if(listOccurredWarnings.size() > 1)
    {
        for (const auto &item : listOccurredWarnings) {
            struct occurredwarning _warning = getOccurredWarningFromQuery(item);
            allOccurredWarnings[_warning.id] = _warning;
        }
    }

    return true;
}

struct occurredwarning OccurredWarningRepository::getOccurredWarningFromQuery(const std::string &query) {
    struct occurredwarning _warning;
    _warning.id = stoi(utils::strings::getValueFromQuery(query, "id", ","));
    _warning.fk_warning = std::stoi(utils::strings::getValueFromQuery(query, "fk_warning", ","));
    _warning.occurredInterface = stoi(utils::strings::getValueFromQuery(query, "fk_hardwareinterface", ","));
    _warning.occurredUser = stoi(utils::strings::getValueFromQuery(query, "fk_user", ","));
    _warning.occurredAt = utils::strings::getValueFromQuery(query, "warningcode", ",");
    _warning.data = utils::strings::getValueFromQuery(query, "warningcodecode", ",");
    _warning.occurred_timestamp = utils::strings::getValueFromQuery(query, "occurred_timestamp", ",");

    return _warning;
}

bool OccurredWarningRepository::logWarning(const struct WarningInfo warningInfo) {
    struct occurredwarning _warning;

    std::string sql = "SELECT id FROM warnings WHERE warningcode = '";
    sql += warningInfo.warnCode;
    sql += "' LIMIT 1;";
    std::string result = databaseService->executeSqlReturn(sql);
    int warning_id = stoi(utils::strings::getValueFromQuery(result, "id", ","));

    sql = "SELECT id FROM users ORDER BY lastlogin DESC LIMIT 1;";
    result = databaseService->executeSqlReturn(sql);
    int user_id = stoi(utils::strings::getValueFromQuery(result, "id", ","));

    _warning.fk_warning = warning_id;
    _warning.occurredInterface = warningInfo.interface;
    _warning.occurredUser = user_id;
    _warning.occurredAt = warningInfo.location;
    _warning.data = warningInfo.data;

    Log::error({
        .message=_warning.data,
        .thrownAt=_warning.occurredAt,
        .loglvl=warn
    });

    return this->persistOccurredWarning(&_warning);
}

std::vector<struct occurredwarning> OccurredWarningRepository::getLastOccurredWarningsByCount(int count) {
    std::vector<struct occurredwarning> ret = {};
    auto it = this->allOccurredWarnings.rbegin();
    while ((it != this->allOccurredWarnings.rend()) && (count > 0)) {
        ret.emplace_back(it->second);
        it++;
        count--;
    }
    std::reverse(ret.begin(), ret.end());

    return ret;
}

std::string OccurredWarningRepository::setWarningResolved(int id, std::string resolved_timestamp)
{
    struct occurredwarning _warning = OccurredWarningRepository::instance->getOccurredWarningById(id);
    std::string sql = "UPDATE occurredwarnings SET resolved_timestamp = '";
    sql += resolved_timestamp + "' WHERE id = ";
    sql += std::to_string(id) + ";";

    this->databaseService->executeSql(sql);

    OccurredWarningRepository::instance->loadAll();

    return resolved_timestamp;
}