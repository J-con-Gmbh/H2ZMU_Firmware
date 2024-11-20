//
// Created by jriessner on 31.05.2022.
//

#include <memory>
#include <map>

#include "vessel/Vessel.h"
#include "data/db/entities/e_HardwareProtocol.h"
#include "data/db/entities/e_Cascade.h"
#include "data/db/repositories/CascadestateRepository.h"
#include "data/db/repositories/CascadeRepository.h"
#include "data/db/repositories/BottleRepository.h"
#include "data/db/repositories/SensorRepository.h"
#include "data/db/repositories/HardwareProtocolRepository.h"

std::shared_ptr<Vessel> Vessel::instance;

vesselstate Vessel::getVesselState() {

    int currentId = VesselstateRepository::instance->getNextValidId();
    struct vesselstate vState {
            .id = currentId,
            .geom_volume_0 = 0,
            .content_kg = 0
    };

    for (auto &item : this->cascades) {
        struct cascadestate cascadeState = item.second.getCascadeState();
        cascadeState.fk_vstate = vState.id;

        vState.geom_volume_0 += cascadeState.geom_volume;
        vState.content_kg += cascadeState.content_kg;
        vState.norm_volume += cascadeState.norm_volume;
        CascadestateRepository::instance->newCascadestate(cascadeState);
        vState.fk_cascadestates.push_back(cascadeState.id);
    }

    return vState;
}

void Vessel::setup() {

    std::map<int, struct cascade> _cascades = CascadeRepository::instance->getAllCascades();
    std::map<int, struct bottle> _bottles = BottleRepository::instance->getAllBottles();
    std::map<int, struct sensor> _sensors = SensorRepository::instance->getAllSensors();
    std::map<int, struct hardwareprotocol> _protocols = HardwareProtocolRepository::instance->getAllProtocols();

    for (const auto &item: _sensors) {
        struct sensor sensor = item.second;
        struct hardwareprotocol protocol   = _protocols[sensor.fk_hardwareprotocol];

        if (sensor.type == 2) {
            std::shared_ptr<PressureSensor> pressureSensor = std::make_shared<PressureSensor>(
                    sensor,
                    sensorspec{
                            .hardwareprotocol = sensor.fk_hardwareprotocol,
                            .hardwareprotocol_address = sensor.hardwareprotocol_address,
                            .inputMax = protocol.rawValueMax,
                            .inputMin = protocol.rawValueMin,
                            .outputMax = sensor.uppermeasurelimit_manufacturer,
                            .outputMin = sensor.lowermeasurelimit_manufacturer
                    });
            this->pressureSensors.insert(std::make_pair(item.first, pressureSensor));
            this->sensorCascadeRelation.insert(std::make_pair(pressureSensor->getId(), item.first));
        } else if (sensor.type == 1) {
            std::shared_ptr<TempSensor> tempSensor = std::make_shared<TempSensor>(
                    sensor,
                    sensorspec{
                            .hardwareprotocol = sensor.fk_hardwareprotocol,
                            .hardwareprotocol_address = sensor.hardwareprotocol_address,
                            .inputMax = protocol.rawValueMax,
                            .inputMin = protocol.rawValueMin,
                            .outputMax = sensor.uppermeasurelimit_manufacturer,
                            .outputMin = sensor.lowermeasurelimit_manufacturer
                    });
            this->tempSensors.insert(std::make_pair(item.first, tempSensor));
        }
    }

    for (const auto &item : _cascades) {
        std::shared_ptr<PressureSensor> pressureSensor;
        std::shared_ptr<PressureSensor> pressureSensorLower = nullptr;

        pressureSensor = this->pressureSensors[item.second.fk_pressure_sensor_upper];

        //if (item.second.fk_pressure_sensor_lower != -1)
        //    pressureSensorLower = this->pressureSensors[item.second.fk_pressure_sensor_lower];

        Cascade cascade(item.first, pressureSensor, pressureSensorLower);
        cascades.insert(std::pair<int, Cascade>(item.first, cascade));
    }


    for (const auto &item : _bottles) {
        this->geometricVolume += item.second.vol_0;

        Bottle bottle;
        bottle.btl = item.second;

        int cascadeId = item.second.fk_cascade;
        int fk_sensor = item.second.fk_sensor;
        if (fk_sensor != -1) {
            struct sensor _sensor = _sensors.find(fk_sensor)->second;
            struct hardwareprotocol protocol = _protocols[_sensor.fk_hardwareprotocol];
            std::shared_ptr<TempSensor> tempSensor = std::make_shared<TempSensor>(_sensor,
                                                                                  sensorspec {
                                              .hardwareprotocol = _sensor.fk_hardwareprotocol,
                                              .hardwareprotocol_address = _sensor.hardwareprotocol_address,
                                              .inputMax = protocol.rawValueMax,
                                              .inputMin = protocol.rawValueMin,
                                              .outputMax = _sensor.uppermeasurelimit_manufacturer,
                                              .outputMin = _sensor.lowermeasurelimit_manufacturer}
                                              );
            tempSensors.insert(std::make_pair(_sensor.id, tempSensor));
            bottle.sensor = tempSensor;

            this->sensorCascadeRelation.insert(std::make_pair(tempSensor->getId(), cascadeId));
        }
        cascades.find(cascadeId)->second.addBottle(bottle);
    }
}

float Vessel::getGeomVolume(bool corrected) {
    if (corrected) {
        float volume = 0;
        for (auto &item: this->cascades) {
            volume += item.second.getCurrentGeometricVolume();
        }
        return volume;
    } else {
        return this->geometricVolume;
    }
}

float Vessel::calcVolExpansion(float pressure_cur, float vol_0, float pressure_0, float vol_ref, float pressure_ref) {
    // f(x) = m * x + c
    float x_0 = pressure_0;
    float y_0 = vol_0;
    // Konstanten der Funktion für die angegebenen Messpunte errechnen
    float m = (vol_ref - vol_0) / (pressure_ref - pressure_0);
    // c = (m * x_0 - y_0) * (-1)
    float c = - (m * x_0) + y_0;

    // Interpoliertes Volumen für angegebenen Druck errechnen
    float vol_cur = m * pressure_cur + c;

    return vol_cur;
}

std::unique_ptr<Cascade> Vessel::getCascadeById(int id) {
    if (this->cascades.count(id) ) {
        return std::make_unique<Cascade>(this->cascades.find(id)->second);
    }
    return {};
}

std::vector<std::shared_ptr<PressureSensor>> Vessel::getPressureSensorsPtr() {
    // TODO bessere lösung ausdenken -> Kopieren eines shared_ptr sehr ressorucen-aufwendig

    std::vector<std::shared_ptr<PressureSensor>> sensors = std::vector<std::shared_ptr<PressureSensor>>();
    for (const auto &item: this->pressureSensors) {
        sensors.emplace_back(item.second);
    }

    return sensors;
}

std::vector<std::shared_ptr<TempSensor>> Vessel::getTempSensorsPtr() {
    // TODO bessere lösung ausdenken -> Kopieren eines shared_ptr sehr ressorucen-aufwendig

    std::vector<std::shared_ptr<TempSensor>> sensors = std::vector<std::shared_ptr<TempSensor>>();
    for (auto &item: this->tempSensors) {
        sensors.emplace_back(item.second);
    }

    return sensors;
}

std::vector<std::unique_ptr<Cascade>> Vessel::getCascadesPtr() {
    std::vector<std::unique_ptr<Cascade>> sensors;
    for (const auto &item: this->cascades) {
        sensors.push_back(std::make_unique<Cascade>(item.second));
    }

    return sensors;
}

std::map<int, int> Vessel::getSensorCascadeRelation() {
    return this->sensorCascadeRelation;
}

