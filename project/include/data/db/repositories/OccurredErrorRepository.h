//
// Created by jriessner on 24.05.2022.
//

#ifndef H2ZMU_2_OCCURREDERRORREPOSITORY_H
#define H2ZMU_2_OCCURREDERRORREPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/DatabaseService.h"
#include "data/db/entities/e_OccurredError.h"

enum hardwareInterface {
    runtime = 0,
    service_software = 1,
    gui = 2,
};

struct ErrorInfo {
    std::string errCode;
    hardwareInterface interface;
    std::string location;
    std::string data;
};

class OccurredErrorRepository: public Repository {
    std::map<int, struct occurrederror> allOccurredErrors;

public:
    static std::shared_ptr<OccurredErrorRepository> instance;

    OccurredErrorRepository() : Repository("occurrederrors") {}
    struct occurrederror getOccurredErrorById(int id);
    std::map<int, struct occurrederror> getAllOccurredErrors();
    bool persistOccurredError(struct occurrederror *_occurrederror);
    std::string setErrorResolved(int id, std::string resolved_timestamp);

    bool loadAll() override;
    static struct occurrederror getOccurredErrorFromQuery(const std::string &query);
    static std::string dumpOccurredError(const struct occurrederror& _error);
    bool logError(const struct ErrorInfo& errorInfo);
    std::vector<struct occurrederror> getLastOccurredErrorsByCount(int count);
};


#endif //H2ZMU_2_OCCURREDERRORREPOSITORY_H
