//
// Created by jriessner on 09.06.2022.
//

#ifndef H2ZMU_2_E_BOTTLE_H
#define H2ZMU_2_E_BOTTLE_H

#include <string>

struct bottle {
    int id = -1;
    std::string serialnumber;
    int fk_cascade = -1;
    int cascade_order = -1;
    int fk_sensor = -1; /// fk for Temp Sensor
    float tara = 0;
    std::string manufacturer;
    int builtyear = -1;
    int nextcheck = -1;
    float vol_0 = 0;
    float pressure_0 = 0;
    float vol_ref = 0;
    float pressure_ref = 0;
    float max_operating_pressure = 0;
};

#endif //H2ZMU_2_E_BOTTLE_H
