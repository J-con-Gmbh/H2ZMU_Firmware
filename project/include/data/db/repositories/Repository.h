//
// Created by jriessner on 20.12.22.
//

#ifndef H2ZMU_2_REPOSITORY_H
#define H2ZMU_2_REPOSITORY_H

#include <memory>

#include "data/db/DatabaseService.h"


// TODO extract loadAll function to base class.

class Repository {
protected:
    std::shared_ptr<DatabaseService> databaseService;
    int currentId = -1;
    std::string tableName;

    int getLastId();

public:

    /**

     TODO get<Entity>FromQuery -> try catch einbauen -> stoi und stof

     */

    explicit Repository(const std::string &tableName);
    virtual void setup(std::shared_ptr<DatabaseService> service);
    virtual bool loadAll() = 0;

    int getNextValidId();
    int getCurrentId();
};

#endif //H2ZMU_2_REPOSITORY_H
