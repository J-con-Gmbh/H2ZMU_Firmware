//
// Created by jriessner on 24.05.2022.
//

#ifndef H2ZMU_2_ERRORREPOSITORY_H
#define H2ZMU_2_ERRORREPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/DatabaseService.h"
#include "data/db/entities/e_Error.h"

class ErrorRepository: public Repository {
    std::map<int, struct error> allErrors;
    std::map<std::string, int> errorCodeIdRel;

public:
    static std::shared_ptr<ErrorRepository> instance;

    ErrorRepository() : Repository("errors") {}
    struct error getErrorById(int id);
    struct error getErrorByCode(const std::string& code);
    std::map<int, struct error> getAllErrors();
    bool loadAll() override;
    static struct error getErrorFromQuery(const std::string& query);
    static std::string dumpError(struct error _error);
    bool createNewError(struct error *_error);
};


#endif //H2ZMU_2_ERRORREPOSITORY_H