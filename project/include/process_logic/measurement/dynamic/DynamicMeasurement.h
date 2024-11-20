//
// Created by jriessner on 25.07.23.
//

#ifndef H2ZMU_V1_DYNAMICMEASUREMENT_H
#define H2ZMU_V1_DYNAMICMEASUREMENT_H

#include "dynamic_measurement_alloc.h"
#include "queue/Queue.h"

namespace msrmnt {

    class DynamicMeasurement {
    private:
        struct dynamic_measurementstate dynamic_state;
        std::map<int, int> pressureSensorCascadeRel;
        std::map<int, int> temperatureSensorCascadeRel;

        uint pressureSensorstateSubscription;
        uint temperatureSensorstateSubscription;
        std::map<int, std::vector<sensorstate>> sensorstateDictionary;

        std::tuple<bool, int, sensordata::data_area> detectSensorTrendChange(const std::map<int, int>& sensorIds);
        std::tuple<bool, int, sensordata::data_area> detectPressureTrendChange();
        std::tuple<bool, int, sensordata::data_area> detectTemperatureTrendChange();
        std::tuple<bool, int, dataset> detectPressureJump();

        void updateSensordataDictionary();

    public:
        std::shared_ptr<DynamicMeasurement> instance;

        DynamicMeasurement();
        std::tuple<bool, struct measurement> handle();
    };

}

#endif //H2ZMU_V1_DYNAMICMEASUREMENT_H
