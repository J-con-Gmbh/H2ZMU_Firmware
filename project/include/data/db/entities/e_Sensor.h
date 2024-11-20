//
// Created by jriessner on 09.06.2022.
//

#ifndef H2ZMU_2_E_SENSOR_H
#define H2ZMU_2_E_SENSOR_H

#include <string>

enum type {
    temp = 1,
    pressure = 2
};

struct sensor {
    int id = -1;
    int type = -1;
    int type_order = -1;
    std::string serialnumber;
    std::string name;
    std::string manufacturer;
    float uppermeasurelimit_manufacturer = -1;
    float lowermeasurelimit_manufacturer = -1;
    float uppermeasurelimit_user = -1;
    float lowermeasurelimit_user = -1;
    float correction_m = -1;
    float correction_b = -1;
    int fk_hardwareprotocol = -1;
    int hardwareprotocol_address = -1;
    time_t calibration_date;
    float offset = 0;
};

#endif //H2ZMU_2_E_SENSOR_H
