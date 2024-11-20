//
// Created by jriessner on 29.08.2022.
//

#include "data/db/repositories/MeasurementRepository.h"

std::shared_ptr<MeasurementRepository> MeasurementRepository::instance;

bool MeasurementRepository::createMeasurement(struct measurement &_measurement) {

    _measurement.id = this->currentId;
    this->currentId += 1;

    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.emplace_back(std::pair<std::string, std::string>("id", std::to_string(_measurement.id)));
    pairs.emplace_back(std::pair<std::string, std::string>("valid","FALSE"));
    pairs.emplace_back(std::pair<std::string, std::string>("ts_start", utils::epoch_time::formatTimestamp(_measurement.timestamp_start)));

    if (!_measurement.external_measurement_id.empty()) {
        pairs.emplace_back(std::pair<std::string, std::string>("ext_measure_id", _measurement.external_measurement_id));
    }
    if (_measurement.user_id != (-1) ) {
        pairs.emplace_back(std::pair<std::string, std::string>("fk_user", std::to_string(_measurement.user_id)));
    }

    std::stringstream sql;
    sql << "insert into measurement (";
    std::stringstream sql2;
    sql2 << ") values (";
    int pairCount = (int) pairs.size();
    for (int i = 0; i < pairCount; i++) {
        std::pair<std::string, std::string> item = pairs.at(i);
        sql << "'" << item.first << "'";
        sql2 << "'" << item.second << "'";
        if (i < pairCount - 1) {
            sql << ", ";
            sql2 << ", ";
        }
    }
    sql << sql2.str() << ");";

    bool ret = this->databaseService->executeSql(sql.str());

    return ret;
}

bool MeasurementRepository::addVesselStateStart(struct measurement& _measurement, const struct vesselstate& vstate_start) {
    std::stringstream sql;
    sql << "update measurement set fk_vesselstate_start = '" << vstate_start.id << "'";
    sql << "where id = " << _measurement.id;

    bool ret = this->databaseService->executeSql(sql.str());
    if (ret) {
        _measurement.fk_vessel_state_start = vstate_start.id;
    }

    return ret;
}

bool MeasurementRepository::addVesselStateEnd(struct measurement& _measurement, const struct vesselstate& vstate_end) {
    std::stringstream sql;
    sql << "update measurement set fk_vesselstate_end = '" << vstate_end.id << "'";
    sql << "where id = " << _measurement.id;

    bool ret = this->databaseService->executeSql(sql.str());
    if (ret) {
        _measurement.fk_vessel_state_end = vstate_end.id;
    }

    return ret;
}

bool MeasurementRepository::finalizeMeasurement(const struct measurement& _measurement) {

    std::stringstream sql;
    sql << "update measurement set valid = '" << (_measurement.valid ? "TRUE" : "FALSE") << "',";
    sql << "ts_end = '" << utils::epoch_time::formatTimestamp(_measurement.timestamp_end) << "'";
    sql << "where id = " << _measurement.id;

    bool success = this->databaseService->executeSql(sql.str());

    if (success) {
        allMeasurements[_measurement.id] = _measurement;
    }

    return success;
}

bool MeasurementRepository::loadAll() {
    std::string sqlmeasurements = "SELECT * FROM measurement;";
    std::string ret = databaseService->executeSqlReturn(sqlmeasurements);
    std::list<std::string> listMeasurements = utils::strings::splitString(ret, ";");

    if(!listMeasurements.empty())
    {
        for (const auto &item : listMeasurements) {
                struct measurement _measurement = getMeasurementFromQuery(item);
                allMeasurements[_measurement.id] = _measurement;
        }
    }

    return true;
}

struct measurement MeasurementRepository::getMeasurementFromQuery(const std::string& query) {
    struct measurement _measurement;
    _measurement.id = stoi(utils::strings::getValueFromQuery(query, "id", ","));
    _measurement.user_id = stoi(utils::strings::getValueFromQuery(query, "fk_user", ","));
    _measurement.timestamp_start = convertStringToTime(utils::strings::getValueFromQuery(query, "ts_start", ","));

    std::string ts_end = utils::strings::getValueFromQuery(query, "ts_end", ",");
    if (ts_end != "-1") {
        _measurement.timestamp_end = convertStringToTime(utils::strings::getValueFromQuery(query, "ts_end", ","));
    }

    _measurement.fk_vessel_state_start = stoi(utils::strings::getValueFromQuery(query, "fk_vesselstate_start", ","));
    _measurement.fk_vessel_state_end = stoi(utils::strings::getValueFromQuery(query, "fk_vesselstate_end", ","));
    _measurement.valid = ("1" == utils::strings::getValueFromQuery(query, "valid", ","));

    return _measurement;
}

struct measurement MeasurementRepository::getLastMeasurement() {
    loadAll();

    struct measurement latest_measurement;
 
    for (int i = 0; i < allMeasurements.size(); i++) {
        if(allMeasurements[i].valid && allMeasurements[i].timestamp_end > latest_measurement.timestamp_end)
        {
            latest_measurement = allMeasurements[i];
        }
    }
    
    return latest_measurement;
}

time_t MeasurementRepository::convertStringToTime(const std::string& timeStr) {

    struct tm tmStruct{};
    strptime(timeStr.c_str(), "%Y-%m-%d %H:%M:%S", &tmStruct);

    return mktime(&tmStruct);
}

struct measurement MeasurementRepository::getMeasurementById(int id) {
    if (this->allMeasurements.count(id)) {
        return this->allMeasurements[id];
    }
    return {};
}

std::vector<struct measurement> MeasurementRepository::getLastMeasurementsByCount(int count) {
    std::vector<struct measurement> ret = {};
    auto it = this->allMeasurements.rbegin();
    while ((it != this->allMeasurements.rend()) && (count > 0)) {   
        ret.emplace_back(it->second);
        it++;
        count--;
    }
    std::reverse(ret.begin(), ret.end());

    return ret;
}
