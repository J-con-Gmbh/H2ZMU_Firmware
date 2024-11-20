//
// Created by jriessner on 29.08.2022.
//

#ifndef H2ZMU_2_MEASUREMENTREPOSITORY_H
#define H2ZMU_2_MEASUREMENTREPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/DatabaseService.h"
#include "data/db/entities/e_Measurement.h"

class MeasurementRepository: public Repository {
private:
    std::map<int, struct measurement> allMeasurements;

    static struct measurement getMeasurementFromQuery(const std::string& query);

public:
    static std::shared_ptr<MeasurementRepository> instance;

    MeasurementRepository() : Repository("measurement") {}
    bool loadAll() override;
    bool createMeasurement(struct measurement &_measurement);
    bool addVesselStateStart(struct measurement& _measurement, const struct vesselstate& vstate_start);
    bool addVesselStateEnd(measurement& _measurement, const vesselstate &vstate_end);
    bool finalizeMeasurement(const measurement& _measurement);
    struct measurement getMeasurementById(int id);
    std::vector<struct measurement> getLastMeasurementsByCount(int count);
    struct measurement getLastMeasurement();
    static time_t convertStringToTime(const std::string& timeStr);
};



#endif //H2ZMU_2_MEASUREMENTREPOSITORY_H
