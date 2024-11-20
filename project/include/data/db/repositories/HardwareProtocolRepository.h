//
// Created by jriessner on 19.06.2022.
//

#ifndef H2ZMU_2_HARDWAREPROTOCOLREPOSITORY_H
#define H2ZMU_2_HARDWAREPROTOCOLREPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/DatabaseService.h"
#include "data/db/entities/e_HardwareProtocol.h"

class HardwareProtocolRepository: public Repository {
    std::map<int, struct hardwareprotocol> allProtocols;

    struct hardwareprotocol getProtocolFromQuery(const std::string &query);

public:
    static std::shared_ptr<HardwareProtocolRepository> instance;

    HardwareProtocolRepository() : Repository("hardwareprotocols") {}
    bool loadAll() override;
    struct hardwareprotocol getProtocolById(int id);
    std::map<int, struct hardwareprotocol> getAllProtocols();

};


#endif //H2ZMU_2_HARDWAREPROTOCOLREPOSITORY_H
