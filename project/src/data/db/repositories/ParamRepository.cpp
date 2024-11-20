//
// Created by jriessner on 17.05.2022.
//

#include <bitset>
#include <iostream>
#include <sstream>

#include "data/db/repositories/ParamRepository.h"

std::shared_ptr<ParamRepository> ParamRepository::instance;

struct param ParamRepository::getParamById(int id) {
    for (const auto &param: allParams)
    {
        if (param.second.id == id)
        {
            return param.second;
        }
    }

    return {};
}


struct param ParamRepository::getParamByNr(int nr) {
    int rel = nrIdRelation[nr];
    struct param p = allParams[rel];
    // TODO Sicherheitsabfragen einbauen
    /*
    int tmp = Security::getCurrentUsergroup();

    if (p.rolerestriction > tmp) {
        return {};
    }
    */

    return p;
}

bool ParamRepository::updateParam(struct param &_param) {

    // TODO Sicherheitsabfragen einbauen
    // TODO Alles außer Value automatisch mit tatächlichen Daten überschreiben
    /*
    if (   ( _param.rolerestriction > Security::getCurrentUsergroup())
        || ( _param.serviceswitch && !Core::buttons.getStatus(button::SERVICE_SWITCH) )
        || ( _param.calibrationswitch && !Core::buttons.getStatus(button::CALIBRATION_SWITCH)
        || ( !this->allParams.count(_param.id) ))
    ) {
        return false;
    }
    _param.timestamp = getTimestamp("%Y-%m-%d %T.%S");
    */

    if (_param == allParams[_param.id]) {
        return true;
    }

    std::string sql = "INSERT INTO paramstate (fk_param, fk_user, timeset, value) "
                      "VALUES ('";

    sql += std::to_string(_param.id) + "', '";
    sql += std::to_string(_param.setBy) + "', '";
    sql += _param.timestamp + "', '";
    sql += _param.value + "');";

    bool success = databaseService->executeSql(sql);
    if (success)
        allParams[_param.id] = _param;

    return success;
}

std::map<int, struct param> ParamRepository::getAllParams() {
    std::map<int, struct param> restrictedParams;

    // TODO Sicherheitsabfragen einbauen
    /*
    int userGroup =Security::getCurrentUsergroup();
    for (const auto &item : allParams) {
        if (item.second.rolerestriction >= userGroup){
            restrictedParams[item.first] = item.second;
        }
    }
    */

    //return restrictedParams;

    return allParams;
}

bool ParamRepository::loadAll() {
    std::string sqlparams = "SELECT * FROM v_params_translations_mapped;";
    std::string sqlparamstate = "SELECT fk_param, fk_user, max(timeset), value from paramstate group by fk_param;";

    std::string retsqlparams = databaseService->executeSqlReturn(sqlparams);
    std::string retsqlparamstate = databaseService->executeSqlReturn(sqlparamstate);

    std::list<std::string> listParams = utils::strings::splitString(retsqlparams, ";");
    std::list<std::string> listParamstate = utils::strings::splitString(retsqlparamstate, ";");

    for (const auto &item : listParamstate) {
        std::string id = utils::strings::getValueFromQuery(item, "fk_param", ",");
        for (const auto &item2 : listParams) {
            std::string id2 = utils::strings::getValueFromQuery(item2, "nr", ",");
            if (id == id2) {
                struct param p = getParamFromQuery(item + "," + item2);
                allParams[p.id] = p;
                listParams.remove(item2);
                break;
            }
        }
    }
    mapNrToId();

    return true;
}

struct param ParamRepository::getParamFromQuery(const std::string &query) {
    struct param _param;

    _param.id           = stoi( utils::strings::getValueFromQuery(query, "id", ","));
    _param.nr           = stoi( utils::strings::getValueFromQuery(query, "nr", ","));
    _param.setBy        = stoi( utils::strings::getValueFromQuery(query, "fk_user", ","));
    _param.timestamp    =           utils::strings::getValueFromQuery(query, "timeset", ",");
    _param.shortdescr   =           utils::strings::getValueFromQuery(query, "shortdescr", ",");
    _param.description  = stoi( utils::strings::getValueFromQuery(query, "fk_description", ","));
    _param.value        =           utils::strings::getValueFromQuery(query, "value", ",");
    _param.datatype     = stoi( utils::strings::getValueFromQuery(query, "datatype", ","));
    _param.unit         =           utils::strings::getValueFromQuery(query, "unit", ",");
    _param.rolerestriction         = stoi(utils::strings::getValueFromQuery(query, "rolerestriction", ","));
    _param.switchrestriction       = stoi(utils::strings::getValueFromQuery(query, "switchrestriction", ","));
    _param.errormsg     = stoi( utils::strings::getValueFromQuery(query, "fk_errormsg", ","));
    _param.hardwareIO   = stoi( utils::strings::getValueFromQuery(query, "fk_hardwareinterface", ","));

    std::bitset<16> bs(_param.switchrestriction);
    _param.serviceswitch = bs.test(0);
    _param.calibrationswitch = bs.test(1);

    return _param;
}

std::string ParamRepository::dumpParam(const struct param& _param) {
    std::stringstream dump;
    dump << "struct param {\n"
              << "\tid:\t\t\t" << _param.id
              << "\n\tshortdescr:\t\t" << _param.shortdescr
              << "\n\tvalue:\t\t\t" << _param.value
              << "\n\tunit:\t\t\t" << _param.unit
              << "\n\tsetBy:\t\t\t" << _param.setBy
              << "\n\tdatatype:\t\t" << _param.datatype
              << "\n\tdescription:\t\t" << _param.description
              << "\n\tserviceswitch:\t\t" << _param.serviceswitch
              << "\n\tcalibrationswitch:\t" << _param.calibrationswitch
              << "\n\tswitchrestriction:\t" << _param.switchrestriction
              << "\n}" << std::endl;

    return dump.str();
}

void ParamRepository::mapNrToId() {
    for (std::pair<int, struct param> item : allParams) {
        nrIdRelation[item.second.nr] = item.first;
    }
}