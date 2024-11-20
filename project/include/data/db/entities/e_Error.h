//
// Created by jriessner on 24.05.2022.
//

#ifndef H2ZMU_2_E_ERROR_H
#define H2ZMU_2_E_ERROR_H

#include <string>

struct error {
    int id;
    std::string errCode;
    int shortDescr; //f_key in Translations
    int possibleCause; //f_key in Translations
    int resolveBy; //f_key in Translations
    bool globalError;
};

#endif //H2ZMU_2_E_ERROR_H
