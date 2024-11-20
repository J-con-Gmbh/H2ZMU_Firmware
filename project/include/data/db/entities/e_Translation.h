//
// Created by jriessner on 18.05.2022.
//

#ifndef H2ZMU_2_E_TRANSLATION_H
#define H2ZMU_2_E_TRANSLATION_H

#include <string>

struct translation {
    int id;
    std::string shrt;
    std::map<std::string, std::string> translations;
};

#endif //H2ZMU_2_E_TRANSLATION_H
