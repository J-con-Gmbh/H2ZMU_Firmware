//
// Created by jriessner on 31.05.2022.
//

#include <thread>
#include "vessel/Sensor.h"
#include "Log.h"
#include "Core.h"
#include "geom/LinearRegression.h"
#include "interface/hart/HartInterface.h"
#include "interface/modules/hart/AXL_HART.h"


// TODO Parametriesieren
#define LIMIT 600

Sensor::Sensor(const struct sensor& _sensor, sensorspec spec) {
    this->_sensor = _sensor;
    this->id = _sensor.id;
    this->serialNr = _sensor.serialnumber;
    this->specs = spec;

    this->protocol = spec.hardwareprotocol;
    this->protocol_address = spec.hardwareprotocol_address;

    this->sensorInputMax = spec.inputMax;
    this->sensorInputMin = spec.inputMin;
    this->sensorOutputMax = spec.outputMax;
    this->sensorOutputMin = spec.outputMin;

    // TODO bessere lösung als hardcoden überlegen
//    this->lastValues = stacked_list<float>(50);
    this->lastValuesTime = std::vector<struct dataset>();

}

std::string Sensor::getSerialNr() {
    return this->serialNr;
}

float Sensor::transformRaw(float rawValue) {

    float sensorValRange = sensorOutputMax - sensorOutputMin;
    float sensorRawRange = sensorInputMax - sensorInputMin;

    float result = (((rawValue - sensorInputMin) * sensorValRange) / sensorRawRange) + sensorOutputMin;

    return result;
}

struct sensorstate Sensor::getSensorState() {
    this->measure();
    auto chilled = this->chilled(20);
    struct sensorstate sstate {
        .fk_sensor = this->id,
        .value = (this->getAvg(AVG_MSR_VALUE) + this->_sensor.offset),
        .value_raw = this->lastSensorValue.value_raw,
        .timestamp = utils::epoch_time::getUnixTimestamp(),
        .chilled = chilled.chilled && chilled.time_elapsed,
        .coeff = chilled.coefficient,
        .seconds_back = chilled.seconds_back,
        .count_sensor_states = chilled.data_eval_count
    };
    lastSensorValue = sstate;

    return sstate;
}

struct sensorstate Sensor::getLastSensorValue() {
    return this->lastSensorValue;
}

int Sensor::getId() const {
    return this->id;
}

float Sensor::getAvg(int count) {
    if (this->lastValuesTime.size() < (count + 1) ) {
        return this->lastSensorValue.value;
    }
    float avg = 0.0;
    for (int i = ( (int)lastValuesTime.size() - count); i < lastValuesTime.size(); ++i) {
        avg += lastValuesTime.at(i).y;
    }
    avg /= (float) count;

    return avg;
}

//TODO Grenzwert, minimale Anzahl an messpunkten, minimale Zeitspanne -> Parametriesierbar
struct ret_type_chilled Sensor::chilled(int secondsBack) {

    struct ret_type_chilled chilled{};

    long now = utils::epoch_time::getUnixTimestamp();
    long until = now - secondsBack;

    std::vector<struct utils::geom::dataset> data;

    for (auto &item: this->lastValuesTime) {
        if (item.x >= until) {
            data.insert(data.end(), {item.x - until, item.y});
            chilled.data_eval_count++;
            if (chilled.seconds_back > item.x || chilled.seconds_back == 0)
                chilled.seconds_back = (int) (now - item.x);
        } else if (!chilled.time_elapsed) {
            chilled.time_elapsed = true;
        }
    }

    utils::geom::LinearRegression regression;
    regression.takeInput(data);
    regression.calculateRegression();
    chilled.coefficient = regression.coefficient();
    //chilled.regression = regression;
    // TODO Parametriesierung des Grenzwertes
    chilled.chilled = (std::abs(chilled.coefficient) < 0.001);

    return chilled;
}

bool Sensor::isChilled(int secondsBack) {
    struct ret_type_chilled ret = this->chilled(secondsBack);

    std::cout << this->id << ": " << ret.data_eval_count << "\t" << ret.seconds_back << "\t" << ret.chilled << "\t" << ret.time_elapsed << "\tKoeffizient: " << ret.coefficient << "\tcapacity: " << this->lastValuesTime.capacity() << std::endl;

    return ret.chilled && ret.time_elapsed;
}

float Sensor::measure() {

    if ( diffSinceLastMeasure() ) {
        this->lastSensorValue.value = this->getData();
        this->lastSensorValue.timestamp = utils::epoch_time::getUnixTimestamp();
        this->lastSensorValue.value_raw = 0.0;
    }

    struct dataset set{this->lastSensorValue.timestamp, this->lastSensorValue.value};

    while ( (! lastValuesTime.empty()) && lastValuesTime.begin()->x < (utils::epoch_time::getUnixTimestamp() - LIMIT)) {
        lastValuesTime.erase(lastValuesTime.begin());
    }
    lastValuesTime.push_back(set);
    //lastValuesTime.shrink_to_fit();
    // TODO Mittelwertbildung überdenken -> unregelmäßige Datenabhoung Lösung: Mittelwertbildung durch Drucktransmitter
    float avg = getAvg(AVG_MSR_VALUE);
    float offset = this->_sensor.offset;

    float value = avg + offset;

    Log::write({
                       .message="Sensor measured, id: " + std::to_string(this->id) + " value: " + std::to_string(value),
                       .thrownAt=__PRETTY_FUNCTION__,
                       .loglvl=loglevel::debug},
               logfile::info);

    return value;
}

float Sensor::smoothenSensorValues() {
    if (this->lastValuesTime.size() < 2) {
        return this->lastSensorValue.value;
    }
    std::vector<struct utils::geom::dataset> data;
    long unixTsNow = utils::epoch_time::getUnixTimestamp();
    long unixTs_10 = unixTsNow - 10;
    for (auto it = this->lastValuesTime.rbegin(); it != this->lastValuesTime.rend(); ++it) {
        if (it->x > unixTs_10) {
            utils::geom::dataset tmp {it->x - unixTs_10, it->y};
            data.push_back(tmp);
        } else {
            break;
        }
    }
    utils::geom::LinearRegression regression;
    regression.takeInput(data);
    regression.calculateRegression();
    float smoothValue = regression.predict(600);

    return smoothValue;
}

bool Sensor::diffSinceLastMeasure() const {
    long diff = utils::epoch_time::getUnixTimestamp() - this->lastSensorValue.timestamp;
    // TODO Parametrisieren
    return (diff >= SENSOR_DATA_MIN_INTERVAL_SEC);
}

float Sensor::getData() {

    float value = -1;
    if (this->protocol == 1) {
        do {
            // TODO breakup condition einbauen -> sonst evtl Endlosschleife
            // TODO Saubere Abholung am Gateway implementiern
            if (value == 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            std::vector<uint16_t> data = HartInterface::getDataFromDevice(this->protocol_address);
            value = HartInterface::mapDataToFloat(data[6], data[7]);
        } while (value == 0);
    } else if (this->protocol == 2) {
        utils::Config conf;
        conf.loadConfigFile(Core::instance->config->getValue("EMULATED_SENSORVALS"));
        std::string tmp = conf.getValue(std::to_string(this->protocol_address));
        try {
            value = std::stof(tmp);
        } catch (std::exception &e) {}
    }

    else if (this->protocol == 4)
    {
        // TODO: HART Protocol : Read Sensorvals from float array. How do I identify the sensor ?
        AXL_HART hart_1("Arp.Plc.Eclr/HART");
        std::tuple<bool, uint16_t, std::vector<float>> hart_handshake;
        hart_handshake = hart_1.receiveData();
        std::vector<float> hart_data = std::get<std::vector<float>>(hart_handshake);
        if(this->id < hart_data.size())
        {
            value = hart_data[this->id];
        }
        else
        {
            value = -1;
        }
        
    }

    Log::write({
                       .message="Raw sensor( " + std::string(this->serialNr) + " ) data updated, id: " + std::to_string(this->id) + " value: " + std::to_string(value),
                       .thrownAt=__PRETTY_FUNCTION__,
                       .loglvl=loglevel::debug},
               logfile::info);

    return value;
}

std::vector<dataset> Sensor::getLastValuesTime() {
    return this->lastValuesTime;
}
