//
// Created by jriessner on 09.06.2022.
//

#include <sstream>
#include <utility>

#include "data/db/repositories/SensorRepository.h"

std::shared_ptr<SensorRepository> SensorRepository::instance;

bool SensorRepository::loadAll() {
    std::string sql = "SELECT * FROM sensors;";

    std::string ret = databaseService->executeSqlReturn(sql);

    size_t pos = 0;
    std::string token;
    while ((pos = ret.find(';')) != std::string::npos) {
        token = ret.substr(0, pos);
        struct sensor _sensor = getSensorFromQuery(token);
        allSensors[_sensor.id] = _sensor;
        if (_sensor.type == type::temp) {
            tempSensors.emplace_back(_sensor.id);
        } else if (_sensor.type == type::pressure) {
            pressureSensors.emplace_back(_sensor.id);
        }
        ret.erase(0, pos + 1);
    }

    return true;
}

struct sensor SensorRepository::getSensorById(int id) {
    for (const auto &sensor: allSensors)
    {
        if (sensor.second.id == id)
        {
            return sensor.second;
        }
    }

    return {};
}

bool SensorRepository::createSensor(struct sensor *_sensor) {

    _sensor->id = this->currentId;
    this->currentId++;

    std::string sql = "INSERT INTO sensors ( id, type, serialnumber, name, manufacturer, uppermeasurelimit_manufacturer, lowermeasurelimit_manufacturer, fk_hardwareprotocol, offset) VALUES (";
    sql += "'" + std::to_string(_sensor->id) + "', '" +
           std::to_string(_sensor->type) + "', '" +
           _sensor->serialnumber + "', '" +
           _sensor->name + "', '" +
           _sensor->manufacturer + "', '" +
           std::to_string(_sensor->uppermeasurelimit_manufacturer) + "', '" +
           std::to_string(_sensor->lowermeasurelimit_manufacturer) + "', '" +
           std::to_string(_sensor->fk_hardwareprotocol) + "', '" +
           std::to_string(_sensor->offset)
           ;
    sql += "');";

    bool ret = databaseService->executeSql(sql);
    if (!ret)
        return false;
    this->allSensors[_sensor->id] = *_sensor;

    return false;
}

bool SensorRepository::deleteSensor(const struct sensor& _sensor) {
    std::string sql = "DELETE FROM sensors WHERE id = '";
    sql += std::to_string(_sensor.id)
            + "';";

    return databaseService->executeSql(sql);
}

std::map<int, struct sensor> SensorRepository::getAllSensors() {
    return allSensors;
}

std::string SensorRepository::dumpSensor(const struct sensor& _sensor) {
    std::stringstream dump;
    dump << "struct user {\n"
         << "\tid:\t\t" << _sensor.id
         << "\n\ttype:\t\t" << _sensor.type
         << "\n\tserialnumber:\t" << _sensor.serialnumber
         << "\n\tname:\t\t" << _sensor.name
         << "\n\tmanufacturer:\t" << _sensor.manufacturer
         << "\n\tuppermeasurelimit_manufacturer:" << _sensor.uppermeasurelimit_manufacturer
         << "\n\tlowermeasurelimit_manufacturer:" << _sensor.lowermeasurelimit_manufacturer
         << "\n\toffset:" << _sensor.offset
         << "\n}" << std::endl;

    return dump.str();
}

struct sensor SensorRepository::getSensorFromQuery(const std::string &query) {
    struct sensor _sensor;
    _sensor.id = std::stoi(utils::strings::getValueFromQuery(query, "id", ","));
    _sensor.type = std::stoi(utils::strings::getValueFromQuery(query, "type", ","));
    _sensor.type_order = std::stoi(utils::strings::getValueFromQuery(query, "type_order", ","));
    _sensor.serialnumber = utils::strings::getValueFromQuery(query, "serialnumber", ",");
    _sensor.name = utils::strings::getValueFromQuery(query, "name", ",");
    _sensor.manufacturer = utils::strings::getValueFromQuery(query, "manufacturer", ",");
    _sensor.uppermeasurelimit_manufacturer = std::stof(utils::strings::getValueFromQuery(query, "uppermeasurelimit_manufacturer", ","));
    _sensor.lowermeasurelimit_manufacturer = std::stof(utils::strings::getValueFromQuery(query, "lowermeasurelimit_manufacturer", ","));
    _sensor.fk_hardwareprotocol = std::stoi(utils::strings::getValueFromQuery(query, "fk_hardwareprotocol", ","));
    _sensor.hardwareprotocol_address = std::stoi(utils::strings::getValueFromQuery(query, "hardwareprotocol_address", ","));
    _sensor.offset = std::stof(utils::strings::getValueFromQuery(query, "offset", ","));

    return _sensor;
}

bool SensorRepository::updateSensor(const sensor &_sensor) {

    std::string sql = "UPDATE sensors SET ";
    int iLength = (int) sql.length();

    struct sensor oldSensor = allSensors[_sensor.id];

    if (_sensor.manufacturer != oldSensor.manufacturer) {
        sql += "manufacturer = '" + _sensor.manufacturer + "', ";
    }
    if (_sensor.serialnumber != oldSensor.serialnumber) {
        sql += "serialnumber = '" + _sensor.serialnumber + "', ";
    }
    if (_sensor.name != oldSensor.name) {
        sql += "name = '" + _sensor.name + "', ";
    }
    if (_sensor.type != oldSensor.type) {
        sql += "type = '" + std::to_string(_sensor.type) + "', ";
    }
    if (_sensor.lowermeasurelimit_manufacturer != oldSensor.lowermeasurelimit_manufacturer) {
        sql += "lowermeasurelimit_manufacturer = '" + std::to_string(_sensor.lowermeasurelimit_manufacturer) + "', ";
    }
    if (_sensor.uppermeasurelimit_manufacturer != oldSensor.uppermeasurelimit_manufacturer) {
        sql += "uppermeasurelimit_manufacturer = '" + std::to_string(_sensor.uppermeasurelimit_manufacturer) + "', ";
    }
    if (_sensor.offset != oldSensor.offset) {
        sql += "offset = '" + std::to_string(_sensor.offset) + "', ";
    }

    bool sqlSuccess;
    if (sql.length() > iLength) {
        sql = utils::strings::trimBy(sql, 2);
        sql += " WHERE id = '" + std::to_string(_sensor.id) + "';";

        sqlSuccess = databaseService->executeSql(sql);
        if (sqlSuccess) {
            allSensors[_sensor.id] = _sensor;
        }

        return sqlSuccess;
    } else {
        return true;
    }
}

std::map<int, struct sensor> SensorRepository::getAllTempSensors() {
    std::map<int, struct sensor> tSensors;
    for (const auto &item : tempSensors) {
        tSensors[item] = allSensors[item];
    }

    return tSensors;
}

std::map<int, struct sensor> SensorRepository::getAllPressureSensors() {
    std::map<int, struct sensor> pSensors;
    for (const auto &item : pressureSensors) {
        pSensors[item] = allSensors[item];
    }

    return pSensors;
}

struct sensor SensorRepository::setSensorId(int id, int new_id) {
    if (getSensorById(id).id != new_id)
    {
        std::string sql = "UPDATE sensors SET id = '";
        sql += std::to_string(new_id) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    SensorRepository::loadAll();

    return getSensorById(id);
}

struct sensor SensorRepository::setSensorSerial(int id, std::string serial) {
    if (getSensorById(id).serialnumber != serial)
    {
        std::string sql = "UPDATE sensor SET serialnumber = '";
        sql += serial + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    SensorRepository::loadAll();

    return getSensorById(id);
}

struct sensor SensorRepository::setSensorLowerLimitManufacturer(int id, float limit) {
    if (getSensorById(id).lowermeasurelimit_manufacturer != limit)
    {
        std::string sql = "UPDATE sensor SET lowermeasurelimit_manufacturer = '";
        sql += std::to_string(limit) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    SensorRepository::loadAll();

    return getSensorById(id);
}

struct sensor SensorRepository::setSensorUpperLimitManufacturer(int id, float limit) {
    if (getSensorById(id).uppermeasurelimit_manufacturer != limit)
    {
        std::string sql = "UPDATE sensor SET uppermeasurelimit_manufacturer = '";
        sql += std::to_string(limit) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    SensorRepository::loadAll();

    return getSensorById(id);
}

struct sensor SensorRepository::setSensorLowerLimitUser(int id, float limit) {
    if (getSensorById(id).lowermeasurelimit_user != limit)
    {
        std::string sql = "UPDATE sensor SET lowermeasurelimit_user = '";
        sql += std::to_string(limit) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    SensorRepository::loadAll();

    return getSensorById(id);
}

struct sensor SensorRepository::setSensorUpperLimitUser(int id, float limit) {
    if (getSensorById(id).uppermeasurelimit_user != limit)
    {
        std::string sql = "UPDATE sensor SET uppermeasurelimit_user = '";
        sql += std::to_string(limit) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    SensorRepository::loadAll();

    return getSensorById(id);
}

struct sensor SensorRepository::setSensorCalibrationDate(int id, std::string calibration_date) {
    std::string sql = "UPDATE sensor SET calibration_date = '";
    sql += calibration_date + "' ";
    sql += "WHERE id = " + std::to_string(id) + " ;";
    bool success = databaseService->executeSql(sql);

    SensorRepository::loadAll();

    return getSensorById(id);
}

struct sensor SensorRepository::setSensorCorrectionM(int id, float correction) {
    if (getSensorById(id).correction_m != correction)
    {
        std::string sql = "UPDATE sensor SET correction_m = '";
        sql += std::to_string(correction) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    SensorRepository::loadAll();

    return getSensorById(id);
}

struct sensor SensorRepository::setSensorCorrectionB(int id, float correction) {
    if (getSensorById(id).correction_b != correction)
    {
        std::string sql = "UPDATE sensor SET correction_b = '";
        sql += std::to_string(correction) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    SensorRepository::loadAll();

    return getSensorById(id);
}