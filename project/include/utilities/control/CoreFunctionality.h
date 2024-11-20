//
// Created by jriessner on 14.08.23.
//

#ifndef H2ZMU_V1_COREFUNCTIONALITY_H
#define H2ZMU_V1_COREFUNCTIONALITY_H

#include <string>

namespace core {
namespace functions {
    bool startMeasurement(std::string data);
}

namespace control {

    enum action {
        MEASUREMENT_START = 1,
        MEASUREMENT_STOP = 2,
    };
}

};


#endif //H2ZMU_V1_COREFUNCTIONALITY_H
