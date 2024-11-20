//
// Created by jriessner on 14.08.23.
//

#include "process_logic/measurement/static/StaticMeasurement.h"

bool StaticMeasurement::startMeasurement(std::string externalId) {

    auto result = msrmnt::create_measurement(0, externalId);
    if ( !std::get<bool>(result)) {
        return false;
    }
    this->msrmnt = std::get<measurement>(result);

    return true;
}

bool StaticMeasurement::stopMeasurement() {
    return msrmnt::end_measurement(this->msrmnt);
}
