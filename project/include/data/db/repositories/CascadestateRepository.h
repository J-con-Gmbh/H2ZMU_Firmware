//
// Created by jriessner on 29.08.2022.
//

#ifndef H2ZMU_2_CASCADESTATEREPOSITORY_H
#define H2ZMU_2_CASCADESTATEREPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/DatabaseService.h"
#include "data/db/entities/e_Cascadestate.h"

class CascadestateRepository: public Repository {
    std::map<int, struct cascadestate> allCascadestates;

public:
    static std::shared_ptr<CascadestateRepository> instance;

    CascadestateRepository() : Repository("cascadestate") {}
    bool loadAll() override;
    struct cascadestate getCascadestateById(int id);
    bool newCascadestate(struct cascadestate& cstate);

    static struct cascadestate getCascadestateFromQuery(const std::string& query);
    static std::string dumpCascadestate(const struct cascadestate& _cascadestate);
    std::map<int, struct cascadestate> getAllCascadestates();
};


#endif //H2ZMU_2_CASCADESTATEREPOSITORY_H