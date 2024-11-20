//
// Created by jriessner on 07.06.2022.
//

#ifndef H2ZMU_2_TEMPSENSOR_H
#define H2ZMU_2_TEMPSENSOR_H

#include "Sensor.h"

#include <vector>
#include "epoch_time.h"
#include "data/db/entities/e_Sensor.h"

class TempSensor : public Sensor {
protected:
    float iGetRawValue();
public:
    explicit TempSensor(const struct sensor& _sensor, sensorspec spec): Sensor(_sensor, spec) {};
    float measure() override;
    float getData() override;
};

#endif //H2ZMU_2_TEMPSENSOR_H
