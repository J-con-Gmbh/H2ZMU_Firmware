//
// Created by jriessner on 19.06.2022.
//

#ifndef H2ZMU_2_CASCADEREPOSITORY_H
#define H2ZMU_2_CASCADEREPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/DatabaseService.h"
#include "data/db/entities/e_Cascade.h"

class CascadeRepository: public Repository{
    std::map<int, struct cascade> allCascades;

    struct cascade getCascadeFromQuery(std::string query);

public:
    static std::shared_ptr<CascadeRepository> instance;

    CascadeRepository() : Repository("cascades") {}
    bool loadAll() override;
    struct cascade getCascadeById(int id);
    bool updateCascade(const struct cascade& _cascade);
    bool createCascade(struct cascade& _cascade);
    bool deleteCascade(const struct cascade& _cascade);
    std::map<int, struct cascade> getAllCascades();
    struct cascade setCascadeId(int id, int new_id);
    struct cascade setCascadePressureSensor(int id, int fk_pressure);
};


#endif //H2ZMU_2_CASCADEREPOSITORY_H