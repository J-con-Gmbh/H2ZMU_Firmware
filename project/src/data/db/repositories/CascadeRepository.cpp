//
// Created by jriessner on 19.06.2022.
//

#include "data/db/repositories/CascadeRepository.h"

std::shared_ptr<CascadeRepository> CascadeRepository::instance;

bool CascadeRepository::loadAll() {

    std::string sql = "SELECT * FROM cascades;";

    std::string ret = databaseService->executeSqlReturn(sql);

    size_t pos = 0;
    std::string token;
    while ((pos = ret.find(';')) != std::string::npos) {
        token = ret.substr(0, pos);
        struct cascade _cascade = getCascadeFromQuery(token);
        allCascades[_cascade.id] = _cascade;
        ret.erase(0, pos + 1);
    }

    return true;
}

struct cascade CascadeRepository::getCascadeById(int id) {
    for (const auto &cascade: allCascades)
    {
        if (cascade.second.id == id)
        {
            return cascade.second;
        }
    }

    return {};
}

bool CascadeRepository::updateCascade(const cascade &_cascade) {
    if (getCascadeById(_cascade.id).fk_pressure_sensor_upper != _cascade.fk_pressure_sensor_upper) {
        std::string sql = "UPDATE cascades SET fk_sensor_upper = '";
        sql += std::to_string(_cascade.fk_pressure_sensor_upper) + "';";
        bool success = databaseService->executeSql(sql);

        return success;
    }


    return false;
}

bool CascadeRepository::createCascade(struct cascade &_cascade) {
    _cascade.id = this->getNextValidId();

    std::string sql = "INSERT INTO cascades ( id, fk_sensor_upper ) VALUES ('";
    sql += std::to_string(_cascade.id) + "', '";
    sql += std::to_string(_cascade.fk_pressure_sensor_upper) + "');";

    bool success = databaseService->executeSql(sql);
    if (success) {
        allCascades[_cascade.id] = _cascade;
    }

    return success;
}

bool CascadeRepository::deleteCascade(const cascade &_cascade) {
    std::string sql = "DELETE FROM cascades WHERE id = '";
    sql += std::to_string(_cascade.id)
           + "';";

    bool success = databaseService->executeSql(sql);

    return success;
}

std::map<int, struct cascade> CascadeRepository::getAllCascades() {
    return allCascades;
}

struct cascade CascadeRepository::getCascadeFromQuery(std::string query) {
    struct cascade _cascade{};
    _cascade.id = std::stoi(utils::strings::getValueFromQuery(query, "id", ","));
    _cascade.fk_pressure_sensor_upper = std::stoi(utils::strings::getValueFromQuery(query, "fk_sensor_upper", ","));

    return _cascade;
}

struct cascade CascadeRepository::setCascadeId(int id, int new_id) {
    if (getCascadeById(id).id != new_id)
    {
        std::string sql = "UPDATE cascades SET id = '";
        sql += std::to_string(new_id) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    CascadeRepository::loadAll();

    return getCascadeById(id);
}

struct cascade CascadeRepository::setCascadePressureSensor(int id, int fk_pressure) {
    if (getCascadeById(id).fk_pressure_sensor_upper != fk_pressure)
    {
        std::string sql = "UPDATE cascades SET fk_sensor_upper = '";
        sql += std::to_string(fk_pressure) + "' ";
        sql += "WHERE id = " + std::to_string(id) + " ;";
        bool success = databaseService->executeSql(sql);
    }

    CascadeRepository::loadAll();

    return getCascadeById(id);
}