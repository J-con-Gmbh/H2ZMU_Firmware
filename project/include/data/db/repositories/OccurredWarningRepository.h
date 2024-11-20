//
// Created by afortino on 15.09.2023.
//

#ifndef H2ZMU_2_OCCURREDWARNINGREPOSITORY_H
#define H2ZMU_2_OCCURREDWARNINGREPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/DatabaseService.h"
#include "data/db/entities/e_OccurredWarning.h"
#include "data/db/repositories/OccurredErrorRepository.h"


struct WarningInfo {
    std::string warnCode;
    hardwareInterface interface;
    std::string location;
    std::string data;
};

class OccurredWarningRepository: public Repository {
    std::map<int, struct occurredwarning> allOccurredWarnings;

public:
    static std::shared_ptr<OccurredWarningRepository> instance;

    OccurredWarningRepository() : Repository("occurredwarnings") {}
    struct occurredwarning getOccurredWarningById(int id);
    std::string setWarningResolved(int id, std::string resolved_timestamp);
    std::map<int, struct occurredwarning> getAllOccurredWarnings();
    bool persistOccurredWarning(struct occurredwarning *_occurredwarning);

    bool loadAll() override;
    static struct occurredwarning getOccurredWarningFromQuery(const std::string &query);
    static std::string dumpOccurredWarning(const struct occurredwarning& _warning);
    bool logWarning(const struct WarningInfo warningInfo);
    std::vector<struct occurredwarning> getLastOccurredWarningsByCount(int count);
};


#endif //H2ZMU_2_OCCURREDWARNINGREPOSITORY_H
