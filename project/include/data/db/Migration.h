//
// Created by jriessner on 13.05.2022.
//

#ifndef H2ZMU_2_MIGRATION_H
#define H2ZMU_2_MIGRATION_H

#include <string>
#include <memory>
#include "conf/Config.h"
#include "DatabaseService.h"

class Migration {
    static std::shared_ptr<utils::Config> conf;
    static std::shared_ptr<DatabaseService> dbService;
public:
    static void init(const std::shared_ptr<utils::Config> &config, const std::shared_ptr<DatabaseService> &databaseService);
    static void removeDbFile(std::string path_to_db_file);
    static void resetMigrations();
    static void migrate();
    static void migrate(const std::string& migrations_directory);
};


#endif //H2ZMU_2_MIGRATION_H
