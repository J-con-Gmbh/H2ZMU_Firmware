//
// Created by jriessner on 09.06.2022.
//

#include "data/db/repositories/BottleRepository.h"
#include <sstream>

std::shared_ptr<BottleRepository> BottleRepository::instance;

bool BottleRepository::loadAll() {
    std::string sql = "SELECT * FROM bottles;";

    std::string ret = databaseService->executeSqlReturn(sql);

    size_t pos;
    std::string token;
    while ((pos = ret.find(';')) != std::string::npos) {
        token = ret.substr(0, pos);
        struct bottle _bottle = getBottleFromQuery(token);
        allBottles[_bottle.id] = _bottle;
        ret.erase(0, pos + 1);
    }

    return true;
}

struct bottle BottleRepository::getBottleById(int id) {
    for (const auto &bottle: allBottles)
    {
        if (bottle.second.id == id)
        {
            return bottle.second;
        }
    }

    return {};
}

bool BottleRepository::createBottle(struct bottle &_bottle) {
    _bottle.id = this->getNextValidId();

    std::string sql = "INSERT INTO bottles ( id, serialnumber, fk_cascade, fk_sensor, tara, manufacturer, builtyear, nextcheck, vol_0, pressure_0, vol_ref, pressure_ref, cascade_order) VALUES (";
    sql += "'" +
           std::to_string(_bottle.id) + "', '" +
           _bottle.serialnumber + "', '" +
           std::to_string(_bottle.fk_cascade) + "', '" +
           std::to_string(_bottle.fk_sensor) + "', '" +
           std::to_string(_bottle.tara) + "', '" +
           _bottle.manufacturer + "', '" +
           std::to_string(_bottle.builtyear) + "', '" +
           std::to_string(_bottle.nextcheck) + "', '" +
           std::to_string(_bottle.vol_0) + "', '" +
           std::to_string(_bottle.pressure_0) + "', '" +
           std::to_string(_bottle.vol_ref) + "', '" +
           std::to_string(_bottle.pressure_ref) + "', '" +
           std::to_string(_bottle.cascade_order);
    sql += "');";

    bool ret = databaseService->executeSql(sql);

    if (ret) {
        this->allBottles[_bottle.id] = _bottle;
    }
    return ret;
}

bool BottleRepository::updateBottle(const bottle &_bottle) {

    std::string sql = "UPDATE bottles SET ";
    int iLength = (int) sql.length();

    struct bottle oldBottle = allBottles[_bottle.id];

    if (_bottle.serialnumber != oldBottle.serialnumber) {
        sql += "serialnumber = '" + _bottle.serialnumber + "', ";
    }
    if (_bottle.fk_cascade != oldBottle.fk_cascade) {
        sql += "fk_cascade = '" + std::to_string(_bottle.fk_cascade) + "', ";
    }
    if (_bottle.fk_sensor != oldBottle.fk_sensor) {
        sql += "fk_sensor = '" + std::to_string(_bottle.fk_sensor) + "', ";
    }
    if (_bottle.tara != oldBottle.tara) {
        sql += "tara = '" + std::to_string(_bottle.tara) + "', ";
    }
    if (_bottle.manufacturer != oldBottle.manufacturer) {
        sql += "manufacturer = '" + _bottle.manufacturer + "', ";
    }
    if (_bottle.builtyear != oldBottle.builtyear) {
        sql += "builtyear = '" + std::to_string(_bottle.builtyear) + "', ";
    }
    if (_bottle.nextcheck != oldBottle.nextcheck) {
        sql += "nextcheck = '" + std::to_string(_bottle.nextcheck) + "', ";
    }
    if (_bottle.vol_0 != oldBottle.vol_0) {
        sql += "vol_0 = '" + std::to_string(_bottle.vol_0) + "', ";
    }
    if (_bottle.pressure_0 != oldBottle.pressure_0) {
        sql += "pressure_0 = '" + std::to_string(_bottle.pressure_0) + "', ";
    }
    if (_bottle.vol_ref != oldBottle.vol_ref) {
        sql += "vol_ref = '" + std::to_string(_bottle.vol_ref) + "', ";
    }
    if (_bottle.pressure_ref != oldBottle.pressure_ref) {
        sql += "pressure_ref = '" + std::to_string(_bottle.pressure_ref) + "', ";
    }

    bool sqlSuccess;
    if (sql.length() > iLength) {
        sql = utils::strings::trimBy(sql, 2);
        sql += " WHERE id = '" + std::to_string(_bottle.id) + "';";

        sqlSuccess = databaseService->executeSql(sql);
        if (sqlSuccess) {
            allBottles[_bottle.id] = _bottle;
        }

        return sqlSuccess;
    } else {
        return true;
    }
}

bool BottleRepository::deleteBottle(const bottle &_bottle) {
    std::string sql = "DELETE FROM bottles WHERE id = '";
    sql += std::to_string(_bottle.id)
           + "';";
    bool success = databaseService->executeSql(sql);
    if (success) {
        this->allBottles.erase(_bottle.id);
    }

    return success;
}

std::map<int, struct bottle> BottleRepository::getAllBottles() {
    return allBottles;
}

std::string BottleRepository::dumpBottle(const bottle &_bottle) {
    std::stringstream dump;
    dump << "struct user {\n"
         << "\tid:\t\t" << _bottle.id
         << "\n\tserialnumber:" << _bottle.serialnumber
         << "\n\tfk_cascade:\t" << _bottle.fk_cascade
         << "\n\tfk_sensor:\t" << _bottle.fk_sensor
         << "\n\ttara:\t\t" << _bottle.tara
         << "\n\tmanufacturer:" << _bottle.manufacturer
         << "\n\tbuiltyear:\t" << _bottle.builtyear
         << "\n\tnextcheck:\t" << _bottle.nextcheck
         << "\n\tvol_0:\t" << _bottle.vol_0
         << "\n\tpressure_0:\t" << _bottle.pressure_0
         << "\n\tvol_ref:\t" << _bottle.vol_ref
         << "\n\tpressure_ref:\t" << _bottle.pressure_ref
         << "\n}" << std::endl;

    return dump.str();
}

struct bottle BottleRepository::getBottleFromQuery(const std::string &query) {
    struct bottle _bottle;
    _bottle.id = std::stoi(utils::strings::getValueFromQuery(query, "id", ","));
    _bottle.serialnumber = utils::strings::getValueFromQuery(query, "serialnumber", ",");
    _bottle.fk_cascade = std::stoi(utils::strings::getValueFromQuery(query, "fk_cascade", ","));
    _bottle.cascade_order = std::stoi(utils::strings::getValueFromQuery(query, "cascade_order", ","));
    _bottle.fk_sensor = std::stoi(utils::strings::getValueFromQuery(query, "fk_sensor", ","));
    _bottle.tara = std::stof(utils::strings::getValueFromQuery(query, "tara", ","));
    _bottle.manufacturer = utils::strings::getValueFromQuery(query, "manufacturer", ",");
    _bottle.builtyear = std::stoi(utils::strings::getValueFromQuery(query, "builtyear", ","));
    _bottle.nextcheck = std::stoi(utils::strings::getValueFromQuery(query, "nextcheck", ","));
    _bottle.vol_0 = std::stof(utils::strings::getValueFromQuery(query, "vol_0", ","));
    _bottle.pressure_0 = std::stof(utils::strings::getValueFromQuery(query, "pressure_0", ","));
    _bottle.vol_ref = std::stof(utils::strings::getValueFromQuery(query, "vol_ref", ","));
    _bottle.pressure_ref = std::stof(utils::strings::getValueFromQuery(query, "pressure_ref", ","));

    return _bottle;
}

struct bottle BottleRepository::setBottleSensor(int new_id, int id) {
    if (getBottleById(id).fk_sensor != id)
    {
        std::string sql = "UPDATE bottles SET fk_sensor = '";
        sql += std::to_string(id) + "' ";
        sql += "WHERE id = " + std::to_string(new_id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    BottleRepository::loadAll();

    return getBottleById(id);
}

struct bottle BottleRepository::setBottleId(int id, int new_id) {
    if (getBottleById(id).id != new_id)
    {
        std::string sql = "UPDATE bottles SET id = '";
        sql += std::to_string(new_id) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    BottleRepository::loadAll();

    return getBottleById(id);
}

struct bottle BottleRepository::setBottleCascade(int id, int cascade_id) {
    if (getBottleById(id).fk_cascade != cascade_id)
    {
        std::string sql = "UPDATE bottles SET fk_cascade = '";
        sql += std::to_string(cascade_id) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    BottleRepository::loadAll();

    return getBottleById(id);
}

struct bottle BottleRepository::setBottleSerial(int id, std::string new_serial) {
    if (getBottleById(id).serialnumber != new_serial)
    {
        std::string sql = "UPDATE bottles SET serialnumber = '";
        sql += new_serial + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    BottleRepository::loadAll();

    return getBottleById(id);
}

struct bottle BottleRepository::setBottleManufacturer(int id, std::string manufacturer) {
    if (getBottleById(id).manufacturer != manufacturer)
    {
        std::string sql = "UPDATE bottles SET manufacturer = '";
        sql += manufacturer + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    BottleRepository::loadAll();

    return getBottleById(id);
}

struct bottle BottleRepository::setBottleBuiltyear(int id, int builtyear) {
    if (getBottleById(id).builtyear != builtyear)
    {
        std::string sql = "UPDATE bottles SET builtyear = '";
        sql += std::to_string(builtyear) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    BottleRepository::loadAll();

    return getBottleById(id);
}

struct bottle BottleRepository::setBottlePressure0(int id, float pressure_0) {
    if (getBottleById(id).pressure_0 != pressure_0)
    {
        std::string sql = "UPDATE bottles SET pressure_0 = '";
        sql += std::to_string(pressure_0) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    BottleRepository::loadAll();

    return getBottleById(id);
}

struct bottle BottleRepository::setBottleVol0(int id, float vol_0) {
    if (getBottleById(id).vol_0 != vol_0)
    {
        std::string sql = "UPDATE bottles SET vol_0 = '";
        sql += std::to_string(vol_0) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    BottleRepository::loadAll();

    return getBottleById(id);
}

struct bottle BottleRepository::setBottlePressureRef(int id, float pressure_ref) {
    if (getBottleById(id).pressure_ref != pressure_ref)
    {
        std::string sql = "UPDATE bottles SET pressure_ref = '";
        sql += std::to_string(pressure_ref) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    BottleRepository::loadAll();

    return getBottleById(id);
}

struct bottle BottleRepository::setBottleVolRef(int id, float vol_ref) {
    if (getBottleById(id).vol_ref != vol_ref)
    {
        std::string sql = "UPDATE bottles SET vol_ref = '";
        sql += std::to_string(vol_ref) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    BottleRepository::loadAll();

    return getBottleById(id);
}