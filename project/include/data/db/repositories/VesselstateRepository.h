//
// Created by jriessner on 29.08.2022.
//

#ifndef H2ZMU_2_VESSELSTATEREPOSITORY_H
#define H2ZMU_2_VESSELSTATEREPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/DatabaseService.h"
#include "data/db/entities/e_Vesselstate.h"

class VesselstateRepository: public Repository {
    std::map<int, struct vesselstate> allVesselstates;

public:
    static std::shared_ptr<VesselstateRepository> instance;

    VesselstateRepository() :Repository("vesselstate") {}

    struct vesselstate createVesselState();
    bool newVesselState(const struct vesselstate& vstate);
    bool updateVesselState(const struct vesselstate& vstate);
    bool loadAll() override;
    struct vesselstate getVesselstate(int id);
    struct vesselstate getVesselstateFromQuery(const std::string &query);
    std::map<int, struct vesselstate> getAllVesselstates();
};

#endif //H2ZMU_2_VESSELSTATEREPOSITORY_H
