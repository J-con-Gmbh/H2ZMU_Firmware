//
// Created by jriessner on 22.06.2022.
//

#include <algorithm>
#include "data/param/ParameterInterface.h"
#include "datatypes.h"
#include "data/db/entities/e_Bottle.h"


ParameterInterface::ParameterInterface(ParamRepository *_paramRepository):
    paramRepository(_paramRepository)
{
    for (const auto &item : this->paramRepository->getAllParams()) {
            paramInParams[item.first];
    }
}

bool ParameterInterface::validateParam(Parameter& parameter) {

    struct param _param = parameter.getParam();

    datatype dtype = (enum datatype) _param.datatype;
    std::string value = _param.value;

    switch (dtype) {
        case datatype::INT:
            try {
                std::stoi(value);
                return true;
            } catch (std::exception &e) {}
            break;
        case datatype::LONG:
            try {
                std::stol(value);
                return true;
            } catch (std::exception &e) {}
            break;
        case datatype::STRING:
            return true;
        case datatype::BOOL:
            return ( (value.find('1') != std::string::npos) && (value.find('0') != std::string::npos) );
        case datatype::FLOAT:
            try {
                std::stof(value);
                return true;
            } catch (std::exception &e) {}
            break;
        case datatype::CHAR:
            return (value.size() == 1);
        default:
            break;
    }
    return false;
}

bool ParameterInterface::updateParam(Parameter &parameter) {
    int nr = parameter.getNr();
    if (parameter.isSavedAsParam()) {

    } else if (101 <= nr && nr <= 102) {

    } else if (201 <= nr && nr <= 250) {

    } else if (251 <= nr && nr <= 300) {

    } else if (301 <= nr && nr <= 500) {

    } else if (501 <= nr && nr <= 600) {

    } else if (601 <= nr) {

    }

    return false;
}

Parameter ParameterInterface::getParameter(int nr) {
/*
    if (paramInParams.count(nr)) {
        // Parameter persistiert in Tabelle params
        return {nr, this->paramRepository->getParamByNr(nr)};
    } else if (101 <= nr && nr <= 103) {
        switch (nr) {
            case 101:
                return {nr, getTimestamp("HH:MM")};
            case 102:
                return {nr, getTimestamp("TT.MM.JJ")};
            case 103:
                // TODO Aktuelle Messnummer zurückgeben
                return {nr, "7"};
            default:
                break;
        }
    } else if (201 <= nr && nr <= 250) {
        // Daten der Druckmessumformer
        int cascadeId = nr % 4;
        if (cascadeId == 0)
            cascadeId = 4;
        auto repo = Core::cascadeRepo.getCascadeById(cascadeId);
        auto sensor = Core::sensorRepo.getSensorById(repo.fk_sensor_lower);

        if (201 <= nr && nr <= 204) {
            return {nr, sensor.name};
        } else if (205 <= nr && nr <= 208) {
            return {nr, sensor.serialnumber};
        } else if (209 <= nr && nr <= 212) {
            return {nr, std::to_string(sensor.lowermeasurelimit)};
        } else if (213 <= nr && nr <= 216) {
            return {nr, std::to_string(sensor.lowermeasurelimit)};
        } else if (217 <= nr && nr <= 220) {
            // TODO Festgelegte (untere) Messgrenze zurückgeben
        } else if (221 <= nr && nr <= 224) {
            // TODO Festgelegte (obere) Messgrenze zurückgeben
        } else if (225 <= nr && nr <= 250) {
            // TODO Kalibrierdaten mit Datum zurückgeben
        }
    } else if (251 <= nr && nr <= 300) {
        auto tempSensors = Core::sensorRepo.getAllTempSensors();
        int entrys = (int) tempSensors.size();
        int start = nr - 250;
        int sensorNr = start % entrys;
        if ( sensorNr == 0 ) sensorNr = entrys;
        struct sensor _sensor;
        for (const auto &item : tempSensors) {
            if (item.second.type_order == sensorNr)
                _sensor = item.second;
        }

        if (start <= entrys) {
            return {nr, _sensor.name};
        } else if (start <= ( entrys * 2 ) ) {
            return {nr, _sensor.serialnumber};
        } else if (start <= ( entrys * 3 ) ) {
            return {nr, std::to_string(_sensor.lowermeasurelimit)};
        } else if (start <= ( entrys * 4 ) ) {
            return {nr, std::to_string(_sensor.uppermeasurelimit)};
        } else if (start <= ( entrys * 2 ) ) {
            // TODO Festgelegte (untere) Messgrenze zurückgeben
        } else if (start <= ( entrys * 2 ) ) {
            // TODO Festgelegte (obere) Messgrenze zurückgeben
        } else if (start <= ( entrys * 2 ) ) {
            // TODO Kalibrierdaten mit Datum zurückgeben
        }
    } else if (301 <= nr && nr <= 500) {
        auto allBottles = Core::bottleRepo.getAllBottles();
        std::map<int, std::map<int, struct bottle>> cascadeBottles;
        for (const auto &item : allBottles) {
            cascadeBottles[item.second.fk_cascade][item.first] = item.second;
        }

        int entrys = (int) allBottles.size();
        int start = nr - 300;
        int fk_cascade = (start / entrys) + 1;
        if ( fk_cascade > 4 )
            fk_cascade - 4;
        int bottleNr = start % entrys;
        if (bottleNr == 0 ) bottleNr = entrys;
        struct bottle _bottle = cascadeBottles[fk_cascade][bottleNr];

        if (301 <= nr && nr <= 380) {
            return {nr, _bottle.serialnumber};
        } else if (401 <= nr && nr <= 480) {
            return {nr, std::to_string(_bottle.volume)};
        }
    } else if (501 <= nr && nr <= 600) {

    } else if (601 <= nr) {

    }
*/

    return {nr, ""};
}
    