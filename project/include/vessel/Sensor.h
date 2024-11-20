//
// Created by jriessner on 31.05.2022.
//

#ifndef H2ZMU_2_SENSOR_H
#define H2ZMU_2_SENSOR_H

#include <string>
#include <vector>

#include "data/db/entities/e_Sensorstate.h"
#include "epoch_time.h"
#include "data/db/entities/e_Sensor.h"

#include "structs.h"

#define AVG_MSR_VALUE 10
#define SENSOR_DATA_MIN_INTERVAL_SEC 1

struct ret_type_chilled {
    bool chilled = false;
    bool time_elapsed = false;
    float coefficient = 0;
    int seconds_back = 0;
    int data_eval_count = 0;
};

struct sensorspec {
    int hardwareprotocol;
    int hardwareprotocol_address;
    float inputMax;
    float inputMin;
    float outputMax;
    float outputMin;
};

struct sensorvalue {
    float value = -1;
    long timestamp = utils::epoch_time::getUnixTimestamp();
};

class Sensor {
protected:
    int id;
    std::string serialNr;
    struct sensor _sensor;
    struct sensorspec specs;
    struct sensorstate lastSensorValue;
//    stacked_list<float> lastValues = stacked_list<float>(0);
    std::vector<dataset> lastValuesTime;

    int protocol;
    int protocol_address;
    float sensorInputMax;
    float sensorInputMin;
    float sensorOutputMax;
    float sensorOutputMin;

    float transformRaw(float rawValue);
    struct ret_type_chilled chilled(int secondsBack);
    float smoothenSensorValues();

    bool diffSinceLastMeasure() const;

public:
    explicit Sensor(const struct sensor& _sensor, sensorspec spec);
    std::string getSerialNr();

    //TODO Beruhigungszeiten implementieren
    virtual float measure();
    virtual float getData();
    float getAvg(int count);
    int getId() const;
    bool isChilled(int secondsBack);

    struct sensorstate getSensorState();
    struct sensorstate getLastSensorValue();
    std::vector<dataset> getLastValuesTime();
};


#endif //H2ZMU_2_SENSOR_H
