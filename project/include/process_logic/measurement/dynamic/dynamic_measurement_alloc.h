//
// Created by jriessner on 25.07.23.
//

#ifndef H2ZMU_V1_DYNAMIC_MEASUREMENT_ALLOC_H
#define H2ZMU_V1_DYNAMIC_MEASUREMENT_ALLOC_H

#include "process_logic/measurement/measurement.h"
#include "structs.h"
#include "process_logic/sensordata/sensordata_trend.h"

namespace msrmnt {

    enum multi_measurementstate {
        idle = 0,   // Pressure is stable
        p_decr = 1, // Pressure is decreasing -> H2 flow
        t_decr = 2, // Temperature decrease in one of the cascades has begun -> map cascade. Occurs parallel to multimsrmntstage::p_decr
        p_wfj = 3,  // Waiting for pressure to jump up -> cycle restarting
    };

    struct dynamic_measurementstate {
        struct measurement measurement;
        bool running = false;
        enum multi_measurementstate state = multi_measurementstate::idle;
        int currentCascadeId = -1;
        ulong currentStartEpoch = 0;
        sensorstate currentStartSensorstatePressure;
        std::vector<sensorstate> currentStartTemperatureSensorstate;
    };

    std::tuple<bool, sensordata::data_area> detect_trend_change(const sensordata::data_trend& trend);
    std::tuple<bool, dataset> detect_first_pressure_jump(sensordata::data_trend trend);

    dataset get_peak_dataset_in_area(const sensordata::data_trend& trend, const sensordata::data_area& area);

}

#endif //H2ZMU_V1_DYNAMIC_MEASUREMENT_ALLOC_H
