//
// Created by afortino on 15.09.2023.
//

#include <sstream>
#include <iostream>

#include "data/db/repositories/WarningRepository.h"

std::shared_ptr<WarningRepository> WarningRepository::instance;

struct warning WarningRepository::getWarningById(int id) {
    for (const auto &warning: allWarnings)
    {
        if (warning.second.id == id)
        {
            return warning.second;
        }
    }

    return {};
}

std::map<int, struct warning> WarningRepository::getAllWarnings() {
    return this->allWarnings;
}

bool WarningRepository::loadAll() {
    std::string sqlwarnings = "SELECT * FROM v_warnings_translations_mapped;";
    std::string ret = databaseService->executeSqlReturn(sqlwarnings);
    std::list<std::string> listWarnings = utils::strings::splitString(ret, ";");

    if(listWarnings.size() > 1)
    {
        for (const auto &item : listWarnings) {
            struct warning _warning = getWarningFromQuery(item);
            allWarnings[_warning.id] = _warning;
        }
    }

    return true;
}

std::string WarningRepository::dumpWarning(struct warning _warning) {
    std::stringstream dump;
    dump << "struct warning {";
    dump << "\n\tid: " << _warning.id;
    dump << "\n\twarningcode: " << _warning.warnCode;
    dump << "\n\tshortdescr: " << _warning.shortDescr;
    dump << "\n\tpossiblecause: " << _warning.possibleCause;
    dump << "\n}";

    return dump.str();
}

struct warning WarningRepository::getWarningFromQuery(const std::string &query) {
    struct warning _warning;
    _warning.id = stoi( utils::strings::getValueFromQuery(query, "id", ","));
    _warning.warnCode = utils::strings::getValueFromQuery(query, "warningcode", ",");
    _warning.shortDescr = stoi( utils::strings::getValueFromQuery(query, "fk_shortdescr", ","));
    _warning.possibleCause = stoi(utils::strings::getValueFromQuery(query, "fk_possiblecause", ","));
    _warning.globalWarning = ( "1" == utils::strings::getValueFromQuery(query, "globalrelevance", ",") );

    return _warning;
}

bool WarningRepository::createNewWarning(struct warning *_warning) {

    _warning->id = this->currentId;
    this->currentId++;

    std::string sql = "INSERT INTO warnings ( id, warningcode, fk_shortdesc, fk_possiblecause, globalrelevance) VALUES (";
    sql += std::to_string(_warning->id) + ", '";
    sql += _warning->warnCode + "', ";
    sql += std::to_string(_warning->shortDescr) + ", ";
    sql += std::to_string(_warning->possibleCause) + ", ";
    sql += std::to_string(_warning->globalWarning) + ");";

    bool ret = databaseService->executeSql(sql);
    if (ret) {
        allWarnings[_warning->id] = *_warning;
    }

    return ret;
}