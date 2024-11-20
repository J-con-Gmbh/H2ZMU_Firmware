//
// Created by jriessner on 07.06.2022.
//

#ifndef H2ZMU_2_PRESSURESENSOR_H
#define H2ZMU_2_PRESSURESENSOR_H

#include "vessel/Sensor.h"
#include "data/db/entities/e_Sensor.h"

class PressureSensor: public Sensor {
protected:
    float iGetRawValue();
public:
    explicit PressureSensor(const struct sensor& _sensor, struct sensorspec spec);
    float measure() override;
    float getData() override;
};


#endif //H2ZMU_2_PRESSURESENSOR_H
