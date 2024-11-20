//
// Created by jriessner on 14.08.23.
//

#ifndef H2ZMU_V1_STATICMEASUREMENT_H
#define H2ZMU_V1_STATICMEASUREMENT_H

#include "process_logic/measurement/measurement.h"

class StaticMeasurement {
private:
    struct measurement msrmnt;
public:
    bool startMeasurement(std::string externalId = "");
    bool stopMeasurement();
};


#endif //H2ZMU_V1_STATICMEASUREMENT_H
