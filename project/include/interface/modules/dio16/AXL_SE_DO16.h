//
// Created by jriessner on 18.12.23.
//

#ifndef H2ZMU_AXCF_AXL_SE_DO16_H
#define H2ZMU_AXCF_AXL_SE_DO16_H


#include <utility>
#include <bitset>

#include "AXL_SE_D16.h"

class AXL_SE_DO16 : public AXL_SE_D16 {
private:
    bool writeToModule();

public:
    explicit AXL_SE_DO16(std::string port);
    bool flush();
    bool setStateOfBit(uint8_t bit, bool state);
    bool unsetAll();
};


#endif //H2ZMU_AXCF_AXL_SE_DO16_H
