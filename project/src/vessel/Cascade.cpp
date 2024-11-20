//
// Created by jriessner on 31.05.2022.
//

#include <cmath>
#include <utility>
#include "vessel/Cascade.h"
#include "vessel/Vessel.h"
#include "data/db/repositories/CascadestateRepository.h"
#include "data/db/repositories/SensorstateRepository.h"
#include "rgfc/RgfCorr.h"

Cascade::Cascade(const int cascadeId, std::shared_ptr<PressureSensor> psensorLower) : pSensorLower(std::move(psensorLower)), pSensorUpper(nullptr) {
    this->id = cascadeId;
}

Cascade::Cascade(const int cascadeId, std::shared_ptr<PressureSensor> psensorLower, std::shared_ptr<PressureSensor> psensorUpper) : pSensorLower(std::move(psensorLower)), pSensorUpper(std::move(psensorUpper)) {
    this->id = cascadeId;
    if (psensorLower) {
        this->hasLowerRangeSensor = true;
    }
}

cascadestate Cascade::getCascadeState() {
    int cStateId = CascadestateRepository::instance->getCurrentId();
    struct sensorstate pSensorValueUpper = this->pSensorLower->getSensorState();

    pSensorValueUpper.fk_cstate = cStateId;
    SensorstateRepository::instance->persistSensorstate(pSensorValueUpper);

    struct sensorstate pSensorValueLower;
    if (this->hasLowerRangeSensor) {
        pSensorValueLower = this->pSensorUpper->getSensorState();
        pSensorValueLower.fk_cstate = cStateId;
        SensorstateRepository::instance->persistSensorstate(pSensorValueLower);
    }
    std::vector<int> tSensorValuesId;

    for (const auto& item : bottles) {
        if (std::shared_ptr<TempSensor> baseTSensor = item.second.sensor) {
            struct sensorstate _sensorstate = baseTSensor->getSensorState();
            _sensorstate.fk_cstate = cStateId;
            SensorstateRepository::instance->persistSensorstate(_sensorstate);
            tSensorValuesId.push_back(_sensorstate.id);
        }
    }

    struct cascadestate cState = {
            .id = cStateId,
            .fk_cascade = this->id,
            .geom_volume = this->geomVolume,
            .geom_volume_corr = getCurrentGeometricVolume(),
            .fk_pressure_lower_sensorstate = pSensorValueLower.id,
            .fk_pressure_upper_sensorstate = pSensorValueUpper.id,
            .fk_temperature_values = tSensorValuesId
    };

    float nm3 = this->getNm3ForCascade(cState);
    cState.norm_volume = nm3;
    cState.content_kg = RgfCorr::instance->convertNormVolToKg(nm3);

    return cState;
}

void Cascade::addBottle(const Bottle& bottle) {
    bottles[bottle.btl.id] = bottle;
    this->geomVolume += bottle.btl.vol_0;
    if (bottle.sensor) {
        tSensors[bottle.btl.id] = bottle.sensor;
    }
}

float Cascade::getCurrentContentKg() {

    float tempCelsius = 0;
    for (const auto &item : tSensors) {
        float tmp = item.second->getLastSensorValue().value;
        tempCelsius += tmp;
    }
    tempCelsius /= (float) tSensors.size();

    float tempKelvin = tempCelsius + (float)273.15;
    float pressure = pSensorLower->getLastSensorValue().value;

    float nm3 = RgfCorr::instance->getNormVolume_m3(this->geomVolume, pressure, tempKelvin);
    float kg = RgfCorr::instance->convertNormVolToKg(nm3);

    return kg;
}

float Cascade::getNm3ForCascade(const cascadestate& cstate) {

    float temp = 0;
    int i = 0;
    for (const auto &item : cstate.fk_temperature_values) {
        auto tmpstate = SensorstateRepository::instance->getSensorstate(item);
        float tmp = tmpstate.value;
        temp += tmp;
        i++;
    }
    temp /= (float) i;
    // TODO Parametriesierte Umrechnungsformel für verschiedene Einheiten der Sensorausgabe ( Kelvin, Celsius, Fahrenheit )

    float tempKelvin = temp + 273.15F;
    float pressure = SensorstateRepository::instance->getSensorstate(cstate.fk_pressure_upper_sensorstate).value;

    float nm3 = RgfCorr::instance->getNormVolume_m3(cstate.geom_volume_corr, pressure, tempKelvin);

    return nm3;
}

std::shared_ptr<PressureSensor> Cascade::getPressureSensorLowerPtr() {
    return this->pSensorLower;
}

float Cascade::getCurrentGeometricVolume() {
    float volume = 0;
    auto lastValue = this->pSensorLower->getLastSensorValue();
    float pressure = lastValue.value;
    if (lastValue.id == -1) {
        pressure = this->pSensorLower->measure();
    }
    for (const auto& item : bottles) {
        struct bottle btl = item.second.btl;
        volume += Vessel::calcVolExpansion(pressure, btl.vol_0, btl.pressure_0, btl.vol_ref, btl.pressure_ref);
    }

    return volume;
}

std::vector<std::shared_ptr<TempSensor>> Cascade::getTemperatureSensorsPtr() {
    std::vector<std::shared_ptr<TempSensor>> ret;
    for (const auto &item: this->tSensors) {
        ret.emplace_back(item.second);
    }

    return ret;
}

int Cascade::getId() const {
    return this->id;
}

std::shared_ptr<PressureSensor> Cascade::getPressureSensorPtr() {
    return this->pSensorUpper;
}
