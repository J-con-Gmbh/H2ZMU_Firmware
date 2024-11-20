//
// Created by jriessner on 25.07.23.
//

#include "process_logic/measurement/dynamic/DynamicMeasurement.h"
#include "process_logic/sensordata/SensorTrendAnalyzer.h"


std::tuple<bool, struct measurement> msrmnt::DynamicMeasurement::handle() {

    this->updateSensordataDictionary();

    std::tuple<bool, int, sensordata::data_area> t_result;
    std::tuple<bool, int, dataset> t_result_jump;
    if (this->dynamic_state.state == multi_measurementstate::idle
        && std::get<bool>( (t_result = this->detectPressureTrendChange()) )
        ) {

        std::cout << "Druckabfall Erkannt" << std::endl;

        std::tuple<bool, struct measurement> t_msrmnt = msrmnt::create_measurement(0 /* TODO User Id einfügen */);

        if (std::get<bool>(t_msrmnt) ) {
            this->dynamic_state.measurement = std::get<struct measurement>(t_msrmnt);
            dataset detectedSet = std::get<sensordata::data_area>(t_result).from;

            std::vector<sensorstate> data = this->sensorstateDictionary[std::get<int>(t_result)];
            sensorstate state;
            for (const auto &set: data) {
                if (set.timestamp > detectedSet.x) {
                    break;
                }
                state = set;
            }
            if (state.id != -1) {
                this->dynamic_state.currentStartSensorstatePressure = state;
            }

            this->dynamic_state.state = p_decr;
        } else {
            // TODO Error implementieren

            std::cerr << "Measurement struct creation failed!" << std::endl;
        }

    } else if ( this->dynamic_state.state == multi_measurementstate::p_decr
        && std::get<bool>( (t_result = this->detectTemperatureTrendChange()) ) /// TODO Unsauber aufgelöst -> bei zwei Detektionen kommt keine Fehlermeldung!!! Problematisch bei mehreren Fühlern pro Kaskade
    ) {

        std::cout << "Temperaturabfall Erkannt" << std::endl;

        sensordata::data_area area = std::get<sensordata::data_area>(t_result);

        int sensorId = this->pressureSensorCascadeRel[std::get<int>(t_result)];

        this->dynamic_state.currentCascadeId = this->temperatureSensorCascadeRel[sensorId];

        std::vector<sensorstate> data = this->sensorstateDictionary[sensorId];
        sensorstate state;
        for (const auto &set: data) {
            if (set.timestamp > area.from.x) {
                break;
            }
            state = set;
        }
        if (state.id != -1) {
            this->dynamic_state.currentStartTemperatureSensorstate.emplace_back(state);
        }
        this->dynamic_state.state = multi_measurementstate::t_decr;

    } else if ( this->dynamic_state.state == multi_measurementstate::t_decr
        && std::get<bool>( (t_result = this->detectPressureTrendChange()) ) /// TODO Unsauber aufgelöst -> nur bei Anstieg/Abfall des Trends, könnte mit Echtdaten für Probleme sorgen
    ) {
        std::cout << "Ende des Druckababfalls erkannt" << std::endl;

        this->dynamic_state.state = multi_measurementstate::p_wfj;
    } else if ( this->dynamic_state.state == multi_measurementstate::p_wfj
        && std::get<bool>( (t_result_jump = this->detectPressureJump()) )
        ) {
        std::cout << "Drucksprung erkannt" << std::endl;

    }

    return {false, measurement{}};
}

msrmnt::DynamicMeasurement::DynamicMeasurement() {

    std::map<int, int> rel = Vessel::instance->getSensorCascadeRelation();
    for (const auto &item: Vessel::instance->getPressureSensorsPtr()) {
        int sensorId = item->getId();
        this->pressureSensorCascadeRel.insert(std::make_pair(sensorId, rel[sensorId]));
    }
    for (const auto &item: Vessel::instance->getTempSensorsPtr()) {
        int sensorId = item->getId();
        this->temperatureSensorCascadeRel.insert(std::make_pair(sensorId, rel[sensorId]));
    }

    this->temperatureSensorstateSubscription = queue::Queues::instance->temperatureSensorState.subscribe();
    this->pressureSensorstateSubscription = queue::Queues::instance->pressureSensorState.subscribe();

}

std::tuple<bool, int, sensordata::data_area> msrmnt::DynamicMeasurement::detectSensorTrendChange(const std::map<int, int>& sensorIds) {

    std::vector<std::tuple<int, bool>> detections;
    int detectionCount = 0;
    sensordata::data_area area;
    int sensorId;

    for (const auto &item: sensorIds) {
        sensordata::data_trend trend = SensorTrendAnalyzer::instance->getTrendForSensor(item.first);
        std::tuple<bool, sensordata::data_area> detection = msrmnt::detect_trend_change(trend);
        bool detect = std::get<bool>(detection);
        area = std::get<sensordata::data_area>(detection);
        detections.emplace_back(std::make_tuple(item.second, detect));
        if (detect) {
            detectionCount += 1;
            sensorId = item.first;
        }
    }

    if (detectionCount > 1) {
        // TODO Globalen Error erzeugen -> GUI anzeige
        /*std::cerr << "Detected change at multiple Sensors. Cascades:";
        for (const auto &item: detections) {
            if (std::get<bool>(item)) {
                std::cerr << " " << std::get<int>(item);
            }
        }
        std::cerr << "!" << std::endl;'
        */
    }

    return std::make_tuple((detectionCount == 1), sensorId, area);
}

std::tuple<bool, int, sensordata::data_area> msrmnt::DynamicMeasurement::detectPressureTrendChange() {
    auto result = this->detectSensorTrendChange(this->pressureSensorCascadeRel);

    return {std::get<bool>(result), -1, std::get<sensordata::data_area>(result)};
}

std::tuple<bool, int, sensordata::data_area> msrmnt::DynamicMeasurement::detectTemperatureTrendChange() {
    return this->detectSensorTrendChange(this->temperatureSensorCascadeRel);
}

std::tuple<bool, int, dataset> msrmnt::DynamicMeasurement::detectPressureJump() {

    std::map<int, bool> detections;
    int sensorId = -1;
    int detectionCount = 0;
    dataset set;

    for (const auto &item: this->pressureSensorCascadeRel) {

        sensordata::data_trend trend = SensorTrendAnalyzer::instance->getTrendForSensor(item.first, 120); /// TODO Zweiter Parameter (Seconds back, default = 300) auf Zeit seit letztem registriertem Sprung setzen
        std::tuple<bool, dataset> detection = msrmnt::detect_first_pressure_jump(trend);
        detections.insert(std::make_pair(item.second, std::get<bool>(detection)));

        set = std::get<dataset>(detection);

        if (std::get<bool>(detection)) {
            detectionCount += 1;
            sensorId = item.first;
        }
    }

    if (detectionCount > 1) {
        // TODO Globalen Error erzeugen -> GUI anzeige

        std::cerr << "Detected increase at multiple Cascades ";
        for (const auto &item: detections) {
            if (std::get<bool>(item)) {
                std::cerr << item.first << " ";
            }
        }
        std::cerr << std::endl;
    }

    return std::make_tuple((detectionCount == 1), sensorId, set);
}

void msrmnt::DynamicMeasurement::updateSensordataDictionary() {
    auto pData = queue::Queues::instance->pressureSensorState.getNew(this->pressureSensorstateSubscription);
    auto tData = queue::Queues::instance->temperatureSensorState.getNew(this->temperatureSensorstateSubscription);

    for (const auto &item: pData) {
        this->sensorstateDictionary[item.fk_sensor].emplace_back(item);
    }
    for (const auto &item: tData) {
        this->sensorstateDictionary[item.fk_sensor].emplace_back(item);
    }

}
