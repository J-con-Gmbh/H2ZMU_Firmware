//
// Created by jriessner on 13.05.2022.
//

#ifndef H2ZMU_2_DATABASESERVICE_H
#define H2ZMU_2_DATABASESERVICE_H

#include <string>
#include <vector>
#include "statement.h"
#include "string_processing.h"
#include "sqlite3.h"

#include <memory>

#define SQL_ERROR "error"
#define SQL_EMPTY "LEER"

class DatabaseService {
    sqlite3 *db;
    std::string filename;
    char *zErrMsg = nullptr;
    int rc;
    const char* data = nullptr;
    sqlite3_mutex *mutex;
    bool _caching = false;
    std::vector<std::string> cachingKeywords = {"INSERT", "ADD", "ALTER", "CREATE", "DELETE", "DROP", "UPDATE", "SET"};

    inline bool iExecuteSql(const std::string&);
    inline std::string iExecuteSqlReturn(const std::string&);

    bool checkCaching(const std::string& stmt);

public:
    //static std::shared_ptr<DatabaseService> instance;
    int getCurrentMigration();
    bool initDb(const std::string &dbFile, bool caching);
    void closeDb();
    bool executeSql(const std::string&);
    std::string executeSqlReturn(const std::string&);
    std::string executeSqlReturnLast(std::vector<std::string> sqlStmts, bool* allQueryOk);

    static int callback(void *NotUsed, int argc, char **argv, char **azColName) {return 0;};

    bool isCacheOnly();
};


#endif //H2ZMU_2_DATABASESERVICE_H
