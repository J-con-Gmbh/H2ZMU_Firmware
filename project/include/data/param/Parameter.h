//
// Created by jriessner on 22.06.2022.
//

#ifndef H2ZMU_2_PARAMETER_H
#define H2ZMU_2_PARAMETER_H


#include "data/db/entities/e_Param.h"

class Parameter {

    int nr;
    struct param _param;
    bool savedAsParam = false;
    std::string value;

public:
    Parameter(int nr, std::string _value);
    Parameter(int nr, struct param _param);

    int getNr() const;
    struct param getParam();
    bool isSavedAsParam();
    std::string getValue();

};


#endif //H2ZMU_2_PARAMETER_H
