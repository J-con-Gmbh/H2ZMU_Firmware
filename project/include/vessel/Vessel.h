//
// Created by jriessner on 31.05.2022.
//

#ifndef H2ZMU_2_VESSEL_H
#define H2ZMU_2_VESSEL_H

#include <map>
#include "Cascade.h"
#include "data/db/entities/e_Vesselstate.h"

class Vessel {
    std::map<int, Cascade> cascades;
    std::map<int, std::shared_ptr<PressureSensor>> pressureSensors;
    std::map<int, std::shared_ptr<TempSensor>> tempSensors;
    std::map<int, int> sensorCascadeRelation;
    float geometricVolume = 0;
    struct vesselstate lastVesselState;

public:
    static std::shared_ptr<Vessel> instance;
    void setup();
    std::map<int, int> getSensorCascadeRelation();
    struct vesselstate getVesselState();
    float getGeomVolume(bool corrected = false);
    std::unique_ptr<Cascade> getCascadeById(int id);
    std::vector<std::unique_ptr<Cascade>> getCascadesPtr();
    std::vector<std::shared_ptr<PressureSensor>> getPressureSensorsPtr();
    std::vector<std::shared_ptr<TempSensor>> getTempSensorsPtr();

    static float calcVolExpansion(float pressure_cur, float vol_0, float pressure_0, float vol_ref, float pressure_ref);
};


#endif //H2ZMU_2_VESSEL_H
