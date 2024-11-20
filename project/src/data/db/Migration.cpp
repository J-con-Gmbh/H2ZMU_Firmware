//
// Created by jriessner on 13.05.2022.
//

#include <filesystem>

#include "data/db/Migration.h"
#include <dirent.h>
#include <iostream>
#include <regex>
#include <chrono>
#include <thread>

#include "sys/FileHandler.h"
#include "Log.h"
#include "Core.h"

std::shared_ptr<utils::Config>             Migration::conf;
std::shared_ptr<DatabaseService>    Migration::dbService;


void Migration::init(const std::shared_ptr<utils::Config> &config, const std::shared_ptr<DatabaseService> &databaseService) {
    Migration::conf = config;
    Migration::dbService = databaseService;
}

void Migration::removeDbFile(std::string path_to_db_file) {
    std::cout << "Reset Database!" << std::endl;
    std::remove(path_to_db_file.c_str());
    std::ofstream file(path_to_db_file.c_str());
}

void Migration::migrate() {
    std::string dir = Migration::conf->getValue("MIGRATIONS_DIR");
    Migration::migrate(dir);
}

void Migration::migrate(const std::string& migrations_directory) {

    // TODO Wenn keine Datenbankdatei vorhanden ist -> SIGABRT

#ifndef NDEBUG
// If build as debug, create option for one time db reset via conf file
    {
        std::string resetConfFileDir = Migration::conf->getValue("FILES_DIR");
        resetConfFileDir = utils::strings::standardiseDirectoryPath(resetConfFileDir) + "reset_db.txt";
        utils::Config reset_db_config;
        if (reset_db_config.loadConfigFile(resetConfFileDir, true) && reset_db_config.isTrue("RESET") ) {
            Migration::removeDbFile(Migration::conf->getValue("DB_FILE"));
        }
        reset_db_config.setValue("RESET", "FALSE");
        reset_db_config.persistConfig();
    }
#endif

    std::string dir = utils::strings::standardiseDirectoryPath(migrations_directory);

    std::vector<struct file> migration_filenames = FileHandler::getFileNamesFromDirAbsPath(dir);

    int migration = Migration::dbService->getCurrentMigration();

    for (const auto &item : migration_filenames) {
        if (item.name.rfind('.', 0) == 0) {
            continue;
        }
        int migration_id = 0;
        std::string string_id = *utils::strings::splitString(item.name, "_").begin();
        try {
            migration_id = std::stoi(string_id);
        } catch (std::exception &e) {

            exit(1);
        }

        if (migration_id > migration) {
            std::string filename = item.path + item.name;
            std::stringstream buffer;
            std::string sql = FileHandler::getFileContent(filename);

            if (sql.empty()) {


            }

            if (Migration::dbService->executeSql(sql)) {
                std::string migrationSql = "INSERT INTO 'migrations' ('version', 'fname') "
                                           "VALUES ('";
                migrationSql += std::to_string(migration_id) + "', '";
                migrationSql += item.name + "');";

                Migration::dbService->executeSql(migrationSql);
                std::cout << "Migration file " << item.name << " successful executed" << std::endl;
            } else {

                std::cout << "ERROR: Migration file " << item.name << std::endl;
            }
        }
    }
}

void Migration::resetMigrations() {
    Migration::dbService->executeSql("delete from migrations;");
}
