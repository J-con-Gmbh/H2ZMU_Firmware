//
// Created by afortino on 15.09.2023.
//

#ifndef H2ZMU_2_WARNINGREPOSITORY_H
#define H2ZMU_2_WARNINGREPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/DatabaseService.h"
#include "data/db/entities/e_Warning.h"

class WarningRepository: public Repository {
    std::map<int, struct warning> allWarnings;

public:
    static std::shared_ptr<WarningRepository> instance;

    WarningRepository() : Repository("warnings") {}
    struct warning getWarningById(int id);
    std::map<int, struct warning> getAllWarnings();
    bool loadAll() override;
    static struct warning getWarningFromQuery(const std::string& query);
    static std::string dumpWarning(struct warning _warning);
    bool createNewWarning(struct warning *_warning);
};


#endif //H2ZMU_2_WARNINGREPOSITORY_H
