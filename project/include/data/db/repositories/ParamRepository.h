//
// Created by jriessner on 17.05.2022.
//

#ifndef H2ZMU_2_PARAMREPOSITORY_H
#define H2ZMU_2_PARAMREPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/DatabaseService.h"
#include "data/db/entities/e_Param.h"


inline bool operator==(const param& lhs, const param& rhs)
{
    return (lhs.id == rhs.id) && (lhs.value == rhs.value) && (lhs.timestamp == rhs.timestamp) && (lhs.setBy == rhs.setBy);
}

class ParamRepository: public Repository {
    std::map<int, struct param> allParams;
    std::map<int, int> nrIdRelation;

    void mapNrToId();

public:
    static std::shared_ptr<ParamRepository> instance;

    ParamRepository() : Repository("params") {}
    struct param getParamById(int id);
    struct param getParamByNr(int nr);
    bool updateParam(struct param &param);
    std::map<int, struct param> getAllParams();
    bool loadAll() override;

    static struct param getParamFromQuery(const std::string& query);
    static std::string dumpParam(const struct param& _param);
    //static std::string getSqlFromParam(const struct param& param);

    /**
     * Returns the param by Nr, if no param with the Nr is present, the id gets checked against all param Id's
     * @param id
     * @return param
     */
    struct param operator[](int id) {
        return this->getParamById(id);
    }
};


#endif //H2ZMU_2_PARAMREPOSITORY_H
