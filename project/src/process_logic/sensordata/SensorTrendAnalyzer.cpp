//
// Created by jriessner on 09.07.23.
//

#include "process_logic/sensordata/SensorTrendAnalyzer.h"

#include "utilities/queue/Queues.h"
#include "vessel/Vessel.h"

std::shared_ptr<SensorTrendAnalyzer> SensorTrendAnalyzer::instance;

void SensorTrendAnalyzer::setup() {
    this->pressureDataQueueSubscription = (int) queue::Queues::instance->pressureSensorData.subscribe();
    this->temperatureDataQueueSubscription = (int) queue::Queues::instance->temperatureSensorData.subscribe();
}

void SensorTrendAnalyzer::update(int sensorId, struct queue::sensor_data data) {
    if (!this->dataTrends.count(sensorId)) {
        int interval = 20;
        std::string sensor_name;
        for (const auto &item: Vessel::instance->getTempSensorsPtr()) {
            if (item->getId() == sensorId) {
                sensor_name = item->getSerialNr();
                interval = 20;
                break;
            }
        }
        for (const auto &item: Vessel::instance->getPressureSensorsPtr()) {
            if (!sensor_name.empty()) {
                break;
            }
            if (item->getId() == sensorId) {
                sensor_name = item->getSerialNr();
                interval = 20;
            }
        }
        this->dataTrends.insert(std::make_pair(sensorId, sensordata::Trend(sensor_name)));
        this->dataTrends[sensorId].setDefaultInterval(interval);
    }

    sensordata::Trend &trend = this->dataTrends[sensorId];
    trend.addData({.x = data.timestamp, .y = data.value});
}

void SensorTrendAnalyzer::updatePressure() {
    std::vector<struct queue::sensor_data> newData = queue::Queues::instance->pressureSensorData.getNew(this->pressureDataQueueSubscription);
    for (const auto &item: newData) {
        this->update(item.sensor_id, item);
    }
}

void SensorTrendAnalyzer::updateTemp() {
    std::vector<struct queue::sensor_data> newData = queue::Queues::instance->temperatureSensorData.getNew(this->temperatureDataQueueSubscription);
    for (const auto &item: newData) {
        this->update(item.sensor_id, item);
    }
}

void SensorTrendAnalyzer::updateAll() {
    updatePressure();
    updateTemp();
}

void SensorTrendAnalyzer::report() {
    for (auto &item: this->dataTrends) {

        //sensordata::trend trend = item.second.getTrend(300).trend;
        //std::cout << trend << std::endl;
        item.second.getTrend(120);
        std::cout << sensordata::Trend::toString(item.second) << std::endl;
    }
}

sensordata::data_trend SensorTrendAnalyzer::getTrendForSensor(int sensorId, int secondsBack) {
    if ( ! this->dataTrends.count(sensorId)) {
        return {};
    }
    sensordata::Trend &trend = std::ref(this->dataTrends[sensorId]);

    return trend.getTrend(secondsBack);
}
