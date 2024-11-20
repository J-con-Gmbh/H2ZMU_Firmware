//
// Created by jriessner on 24.05.2022.
//

#include <sstream>
#include <iostream>

#include "data/db/repositories/ErrorRepository.h"

std::shared_ptr<ErrorRepository> ErrorRepository::instance;

struct error ErrorRepository::getErrorById(int id) {
    for (const auto &error: allErrors)
    {
        if (error.second.id == id)
        {
            return error.second;
        }
    }

    return {};
}

std::map<int, struct error> ErrorRepository::getAllErrors() {
    return this->allErrors;
}

bool ErrorRepository::loadAll() {
    std::string sqlerrors = "SELECT * FROM v_errors_translations_mapped;";
    std::string ret = databaseService->executeSqlReturn(sqlerrors);
    std::list<std::string> listErrors = utils::strings::splitString(ret, ";");

    if(listErrors.size() > 1)
    {
        for (const auto &item : listErrors) {
            struct error _error = getErrorFromQuery(item);
            this->allErrors[_error.id] = _error;
            this->errorCodeIdRel[_error.errCode] = _error.id;
        }
    }

    return true;
}

std::string ErrorRepository::dumpError(struct error _error) {
    std::stringstream dump;
    dump << "struct error {";
    dump << "\n\tid: " << _error.id;
    dump << "\n\terrorcode: " << _error.errCode;
    dump << "\n\tshortdescr: " << _error.shortDescr;
    dump << "\n\tpossiblecause: " << _error.possibleCause;
    dump << "\n\tresolveby: " << _error.resolveBy;
    dump << "\n}";

    return dump.str();
}

struct error ErrorRepository::getErrorFromQuery(const std::string &query) {
    struct error _error;
    _error.id = stoi( utils::strings::getValueFromQuery(query, "id", ","));
    _error.errCode = utils::strings::getValueFromQuery(query, "errorcode", ",");
    _error.shortDescr = stoi( utils::strings::getValueFromQuery(query, "fk_shortdescr", ","));
    _error.resolveBy = stoi( utils::strings::getValueFromQuery(query, "fk_fixby", ","));
    _error.possibleCause = stoi(utils::strings::getValueFromQuery(query, "fk_possiblecause", ","));
    _error.globalError = ( "1" == utils::strings::getValueFromQuery(query, "globalrelevance", ",") );

    return _error;
}

bool ErrorRepository::createNewError(struct error *_error) {

    _error->id = this->currentId;
    this->currentId++;

    std::string sql = "INSERT INTO errors ( id, errorcode, fk_shortdesc, fk_fixby, fk_possiblecause, globalrelevance) VALUES (";
    sql += std::to_string(_error->id) + ", '";
    sql += _error->errCode + "', ";
    sql += std::to_string(_error->shortDescr) + ", ";
    sql += std::to_string(_error->resolveBy) + ", ";
    sql += std::to_string(_error->possibleCause) + ", ";
    sql += std::to_string(_error->globalError) + ");";

    bool ret = databaseService->executeSql(sql);
    if (ret) {
        allErrors[_error->id] = *_error;
    }

    return ret;
}

struct error ErrorRepository::getErrorByCode(const std::string& code) {
    if (this->errorCodeIdRel.count(code)) {
        return this->allErrors[this->errorCodeIdRel[code]];
    }
    return {};
}