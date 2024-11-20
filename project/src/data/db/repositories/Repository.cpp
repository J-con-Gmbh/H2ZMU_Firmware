//
// Created by jriessner on 20.12.22.
//

#include "data/db/repositories/Repository.h"

#include <utility>

void Repository::setup(std::shared_ptr<DatabaseService> service) {
    this->databaseService = std::move(service);
    this->currentId = getLastId() + 1;

    this->loadAll();
}

int Repository::getLastId() {
    std::string sql = "select max(id) as id from ";
    sql += this->tableName;
    sql += ";";
    std::string ret = this->databaseService->executeSqlReturn(sql);
    int lastId = std::stoi(utils::strings::getValueFromQuery(ret, "id", ","));

    return lastId;
}

Repository::Repository(const std::string &tableName) {
    this->tableName = tableName;
}

int Repository::getNextValidId() {
    int id = this->currentId;
    this->currentId += 1;

    return id;
}

int Repository::getCurrentId() {
    return this->currentId;
}