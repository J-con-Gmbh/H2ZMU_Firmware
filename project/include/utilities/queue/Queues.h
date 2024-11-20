//
// Created by jriessner on 06.07.23.
//

#ifndef H2ZMU_V1_QUEUES_H
#define H2ZMU_V1_QUEUES_H

#include <memory>

#include "data/db/entities/e_Sensorstate.h"
#include "interface/hmi/Button.h"
#include "queue/Queue.h"

namespace queue {

    struct sensor_data {
        int sensor_id;
        long timestamp;
        float value;
    };

    class Queues {
    public:
        static std::shared_ptr<Queues> instance;

        utils::Queue<struct sensor_data> temperatureSensorData = utils::Queue<sensor_data>();
        utils::Queue<struct sensor_data> pressureSensorData = utils::Queue<sensor_data>();

        utils::Queue<struct sensorstate> temperatureSensorState = utils::Queue<sensorstate>();
        utils::Queue<struct sensorstate> pressureSensorState = utils::Queue<sensorstate>();

        utils::Queue<enum button> buttonsPressed = utils::Queue<enum button>();
    };

}


#endif //H2ZMU_V1_QUEUES_H
