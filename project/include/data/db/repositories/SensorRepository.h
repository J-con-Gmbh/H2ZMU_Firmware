//
// Created by jriessner on 09.06.2022.
//

#ifndef H2ZMU_2_SENSORREPOSITORY_H
#define H2ZMU_2_SENSORREPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/DatabaseService.h"
#include "data/db/entities/e_Sensor.h"

class SensorRepository: public Repository {
    std::map<int, struct sensor> allSensors;
    std::vector<int> pressureSensors;
    std::vector<int> tempSensors;

    struct sensor getSensorFromQuery(const std::string& query);

public:
    static std::shared_ptr<SensorRepository> instance;

    SensorRepository() : Repository("sensors") {}
    bool loadAll() override;
    struct sensor getSensorById(int id);
    //struct sensor getSensorByName(const std::string& name);
    bool updateSensor(const struct sensor& _sensor);
    bool createSensor(struct sensor *_sensor);
    bool deleteSensor(const struct sensor& _sensor);
    std::map<int, struct sensor> getAllSensors();
    std::map<int, struct sensor> getAllTempSensors();
    std::map<int, struct sensor> getAllPressureSensors();
    static std::string dumpSensor(const struct sensor& _sensor);
    struct sensor setSensorId(int id, int new_id);
    struct sensor setSensorSerial(int id, std::string serial);
    struct sensor setSensorLowerLimitManufacturer(int id, float limit);
    struct sensor setSensorUpperLimitManufacturer(int id, float limit);
    struct sensor setSensorLowerLimitUser(int id, float limit);
    struct sensor setSensorUpperLimitUser(int id, float limit);
    struct sensor setSensorCalibrationDate(int id, std::string calibration_date);
    struct sensor setSensorCorrectionM(int id, float correction);
    struct sensor setSensorCorrectionB(int id, float correction);

};


#endif //H2ZMU_2_SENSORREPOSITORY_H