//
// Created by afortino on 16.04.2024.
//

#ifndef H2ZMU_2_OCCURREDSTATUSEPOSITORY_H
#define H2ZMU_2_OCCURREDSTATUSEPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/DatabaseService.h"
#include "data/db/entities/e_OccurredStatus.h"


class OccurredStatusRepository: public Repository {
    std::map<int, struct occurredstatus> allOccurredStatus;

public:
    static std::shared_ptr<OccurredStatusRepository> instance;

    OccurredStatusRepository() : Repository("occurredstatus") {}
    struct occurredstatus getOccurredStatusById(int id);
    std::map<int, struct occurredstatus> getAllOccurredStatus();
    bool persistOccurredStatus(struct occurredstatus *_occurredstatus);

    bool loadAll() override;
    static struct occurredstatus getOccurredStatusFromQuery(const std::string &query);
    static std::string dumpOccurredStatus(const struct occurredstatus& _status);
    std::vector<struct occurredstatus> getLastOccurredStatusByCount(int count);
};


#endif //H2ZMU_2_OCCURREDSTATUSREPOSITORY_H
