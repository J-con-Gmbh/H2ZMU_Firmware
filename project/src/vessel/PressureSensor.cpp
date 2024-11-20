//
// Created by jriessner on 07.06.2022.
//

#include "vessel/PressureSensor.h"

PressureSensor::PressureSensor(const sensor &_sensor, struct sensorspec spec) : Sensor(_sensor, spec) {
    this->specs = spec;
}

float PressureSensor::measure() {
    return Sensor::measure();
}

float PressureSensor::getData() {
    return Sensor::getData();
}

float PressureSensor::iGetRawValue() {
    return 0;
}
