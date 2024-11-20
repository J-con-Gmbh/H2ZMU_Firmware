//
// Created by jriessner on 09.07.23.
//

#ifndef H2ZMU_V1_SENSORTRENDANALYZER_H
#define H2ZMU_V1_SENSORTRENDANALYZER_H


#include <map>
#include <memory>

#include "sensordata_trend.h"
#include "vessel/PressureSensor.h"
#include "vessel/TempSensor.h"
#include "utilities/queue/Queues.h"

class SensorTrendAnalyzer {
private:
    int pressureDataQueueSubscription = -1;
    int temperatureDataQueueSubscription = -1;

    std::map<int, sensordata::Trend> dataTrends;

    void update(int sensorId, struct  queue::sensor_data data);

public:
    static std::shared_ptr<SensorTrendAnalyzer> instance;

    void setup();
    void updateAll();
    void updatePressure();
    void updateTemp();
    sensordata::data_trend getTrendForSensor(int sensorId, int secondsBack = 300);

    void report();
};


#endif //H2ZMU_V1_SENSORTRENDANALYZER_H
