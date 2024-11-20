//
// Created by jriessner on 22.06.23.
//

#ifndef H2ZMU_V1_MEASUREMENT_H
#define H2ZMU_V1_MEASUREMENT_H

#include <string>

#include "data/db/entities/e_Measurement.h"
#include "data/db/repositories/MeasurementRepository.h"

#include "vessel/Vessel.h"
#include "Log.h"

namespace msrmnt {

    std::tuple<bool, struct measurement> create_measurement (int userId, const std::string& externalId = {});

    bool start_measurement (struct measurement& _measurement);

    bool end_measurement (struct measurement& _measurement);

    bool validate_measurement(struct measurement& _measurement);

    bool set_start_point_for_cascade(struct measurement& _measurement, int cascadeId);

    bool set_end_point_for_cascade(struct measurement& _measurement, int fk_cascade);

    bool set_point_for_cascade(int fk_vesselstate, int cascadeId);

    bool finalize_measurement(struct measurement& _measurement);

    bool persist_measurement(struct measurement& _measurement);

    std::string dump_measurement(const struct measurement& _measurement);

}

#endif //H2ZMU_V1_MEASUREMENT_H
