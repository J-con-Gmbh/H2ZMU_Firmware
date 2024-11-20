//
// Created by jriessner on 22.06.2022.
//

#ifndef H2ZMU_2_PARAMETERINTERFACE_H
#define H2ZMU_2_PARAMETERINTERFACE_H

#include "data/db/entities/e_Param.h"
#include "data/db/repositories/ParamRepository.h"
#include "Parameter.h"
#include <map>
#include <vector>

class ParameterInterface {
    ParamRepository *paramRepository;
    std::map<int, int> paramInParams;

public:
    ParameterInterface(ParamRepository *_paramRepository);
    static bool validateParam(Parameter&);
    bool updateParam(Parameter&);

    bool updatePressureSensorData(Parameter& parameter);
    bool updateTempSensorData(Parameter& parameter);
    Parameter getParameter(int nr);

};


#endif //H2ZMU_2_PARAMETERINTERFACE_H
