//
// Created by jriessner on 13.05.2022.
//

#ifndef H2ZMU_2_E_PARAM_H
#define H2ZMU_2_E_PARAM_H


#include <string>

struct param {
    int id = -1;
    int nr;
    int setBy;
    std::string timestamp;
    std::string shortdescr;
    int description; // fk for translation table
    std::string value;
    int datatype;
    std::string unit;
    int rolerestriction;
    int switchrestriction;
    bool serviceswitch;
    bool calibrationswitch;
    int errormsg; // fKey for error/warn msg table
    int hardwareIO; // fKey for hardwareIO Table
};



/*
class Param {
public:
    std::string setBy;
    long unixTs;
    int id;
    std::string shortDescr;
    int description; // fKey for translation table
    std::string value;
    int dataType;
    std::string unit;
    int role;
    int errormsg; // fKey for error/warn msg table
    int hardwareIO; // fKey for hardwareIO Table
};
*/

#endif //H2ZMU_2_E_PARAM_H
