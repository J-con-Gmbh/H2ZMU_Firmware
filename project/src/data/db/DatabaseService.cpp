//
// Created by jriessner on 13.05.2022.
//

#include "data/db/DatabaseService.h"

#include <iostream>
#include "Log.h"
#include "sys/dump_stacktrace.h"

//std::shared_ptr<DatabaseService> DatabaseService::instance;

using namespace h2zmu;

bool DatabaseService::initDb(const std::string &dbFile, bool caching) {
    this->_caching = caching;
    std::string dir = dbFile.substr(0, dbFile.find_last_of('/'));
    std::stringstream touchFilesCmd;
    touchFilesCmd << "[ -d " << dir << " ] || mkdir -p " << dir;
    system(touchFilesCmd.str().c_str());

    this->filename = dbFile;
    if (sqlite3_initialize() != SQLITE_OK) {
        std::cerr << "sqlite3 initialisierung fehlgeschlagen" << std::endl;
        return false;
    }

    rc = sqlite3_open(this->filename.c_str(), &db);
    if( rc != SQLITE_OK ) {



        sqlite3_free(zErrMsg);

        return false;
    }
    mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);

    Log::write({
                       .message="DatenbankService erfolgreich initialisiert",
                       .thrownAt=__PRETTY_FUNCTION__
               });

    return true;
}

void DatabaseService::closeDb() {
    sqlite3_close(db);
}

bool DatabaseService::iExecuteSql(const std::string& sql) {

    /// TODO Transactions implementieren

    if (checkCaching(sql)) {
        return true;
    }
    bool success = true;
    std::list<std::string> arr = utils::strings::splitString(sql, ";");

    for (const auto &item : arr) {

        bool partial_success = true;

        if (item.empty()) {
            continue;
        }
        std::cout << item << std::endl;
        std::string part_sql = item + ";";
        rc = sqlite3_exec(db, part_sql.c_str(), nullptr, (void *) data, &zErrMsg);
        //rc = sqlite3_exec(db, sql.c_str(), nullptr, (void *)data, &zErrMsg);
        if (rc != SQLITE_OK) {

            sqlite3_mutex_leave(this->mutex);
            partial_success = false;
            success = false;
        }

        if (partial_success) {
            Log::write({
                               .message="SQL: \"" + part_sql + "\" erfolgreich ausgeführt",
                               .thrownAt=__PRETTY_FUNCTION__,
                       }, logfile::sql);
        } else {

        }
    }
    sqlite3_free(zErrMsg);

    return success;
}

bool DatabaseService::executeSql(const std::string& sql) {
/*
    int rc;
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Log::error("!SQL: \"" + sql.substr(0, sql.find('\n')) + "\" Ausführung fehlgeschlagen");
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW);

    return true;
*/
    bool retVal;
    sqlite3_mutex_enter(this->mutex);
    retVal = iExecuteSql(sql);
    sqlite3_mutex_leave(this->mutex);

    return retVal;
}

std::string DatabaseService::iExecuteSqlReturn(const std::string& sql) {

    std::cout << "Hello ?" << std::endl;
    std::cout << sql << std::endl;
    if (utils::strings::splitString(sql, ";").size() > 1) {

        return SQL_ERROR;
    }

    std::string ret;

    if (checkCaching(sql)) {
        return ret;
    }


    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);

    if( rc != SQLITE_OK ) {
        Log::error({
            .message="!SQL: \"" + sql + "\" Ausführung fehlgeschlagen",
            .thrownAt=__PRETTY_FUNCTION__,
            .loglvl=loglevel::failure
        });
        std::cout << "ERROR MSG: " << sqlite3_errmsg(db) << std::endl;
        std::cout << utils::strings::splitString(sql, ";").size() << std::endl;
        return SQL_ERROR;
    }

    Log::write({
                       .message="SQL: \"" + sql + "\" erfolgreich ausgeführt",
                       .thrownAt=__PRETTY_FUNCTION__
               }, logfile::sql);

    int cols = sqlite3_column_count(stmt);
    std::string col_name;
    std::string col_value;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        for (int i=0;i<cols;i++)
        {
            if(sqlite3_column_name(stmt,i))
                col_name = const_cast<const char*>(sqlite3_column_name(stmt,i));
            else col_name = SQL_EMPTY;

            if (sqlite3_column_text(stmt,i))
                col_value = reinterpret_cast<const char*>(sqlite3_column_text(stmt,i));
            else col_value = SQL_EMPTY;

            ret.append(col_name)
                .append("=")
                .append(col_value)
                .append(",");
        }
        ret = utils::strings::trimBy(ret, 1).append(";");
    }

    return ret;
}

std::string DatabaseService::executeSqlReturn(const std::string& sql) {

    std::string retVal;
    sqlite3_mutex_enter(this->mutex);

    retVal = iExecuteSqlReturn(sql);
    sqlite3_mutex_leave(this->mutex);

    return retVal;
}

int DatabaseService::getCurrentMigration() {

    std::string version = executeSqlReturn("SELECT max(version) FROM migrations;");

    if ( version.empty() || (version.find("error") != std::string::npos) || (version.find("LEER") != std::string::npos) )
        return -1;

    version = utils::strings::getValueFromQuery(version, "max(version)", ",");

    return std::stoi(version);
}

std::string DatabaseService::executeSqlReturnLast(std::vector<std::string> sqlStmts, bool* allQueryOk) {
    *allQueryOk = true;
    std::string retVal;
    auto size = sqlStmts.size();
    auto _mutex = this->mutex;
    sqlite3_mutex_enter(_mutex);
    for (int i = 0; i < size; ++i) {
        if (size < i - 1) {
            if (! iExecuteSql(sqlStmts[i])) {
                *allQueryOk = false;
            }
        } else {
            retVal = iExecuteSqlReturn(sqlStmts[i]);
        }
    }
    sqlite3_mutex_leave(this->mutex);

    return retVal;
}

bool DatabaseService::checkCaching(const std::string& stmt) {

    if (!this->_caching) {
        return false;
    }
    /*
    if ( std::any_of(this->cachingKeywords.begin(), this->cachingKeywords.end(),
                     [&stmt](const std::string& s) { return findStringIC(stmt, s);}) )
        return true;
    */
    for (const auto &item: this->cachingKeywords) {
        if (utils::strings::findStringIC(stmt, item)) {
            return true;
        }
    }

    return false;
}

bool DatabaseService::isCacheOnly() {
    return this->_caching;
}
