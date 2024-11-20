//
// Created by jriessner on 31.05.2022.
//

#ifndef H2ZMU_2_CASCADE_H
#define H2ZMU_2_CASCADE_H

#include <memory>

#include <list>
#include <map>
#include <utility>
#include "Bottle.h"
#include "PressureSensor.h"
#include "TempSensor.h"
#include "data/db/entities/e_Sensorstate.h"
#include "data/db/entities/e_Cascadestate.h"

class Vessel;

class Cascade {
    int id;
    float geomVolume = 0;
    std::shared_ptr<PressureSensor> pSensorLower;
    std::shared_ptr<PressureSensor> pSensorUpper;
    bool hasLowerRangeSensor = false;
    std::map<int, std::shared_ptr<TempSensor>> tSensors;
    std::map<int, Bottle> bottles;

    float getNm3ForCascade(const cascadestate& cstate);

public:
    explicit Cascade(int cascadeId, std::shared_ptr<PressureSensor> psensorLower);
    explicit Cascade(int cascadeId,  std::shared_ptr<PressureSensor> psensorLower,  std::shared_ptr<PressureSensor> psensorUpper);

    void addBottle(const Bottle& bottle);
    int getId() const;
    cascadestate getCascadeState();
    float getCurrentGeometricVolume();
    std::shared_ptr<PressureSensor> getPressureSensorPtr();
    std::shared_ptr<PressureSensor> getPressureSensorLowerPtr();
    std::vector<std::shared_ptr<TempSensor>> getTemperatureSensorsPtr();

    float getCurrentContentKg();
};

#endif //H2ZMU_2_CASCADE_H
