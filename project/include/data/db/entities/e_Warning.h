//
// Created by afortino on 15.09.2023.
//

#ifndef H2ZMU_2_E_WARNING_H
#define H2ZMU_2_E_WARNING_H

#include <string>

struct warning {
    int id;
    std::string warnCode;
    int shortDescr; //f_key in Translations
    int possibleCause; //f_key in Translations
    bool globalWarning;
};

#endif //H2ZMU_2_E_WARNING_H
