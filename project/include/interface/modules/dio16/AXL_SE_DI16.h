//
// Created by jriessner on 18.12.23.
//

#ifndef H2ZMU_AXCF_AXL_SE_DI16_H
#define H2ZMU_AXCF_AXL_SE_DI16_H


#include <utility>

#include "AXL_SE_D16.h"

class AXL_SE_DI16 : public AXL_SE_D16 {
public:
    explicit AXL_SE_DI16(std::string port) : AXL_SE_D16(std::move(port)) {};

    /**
     * This function calls AXL_SE_D16::readFromModule() function, and bypasses the onHold directive
     * @return success of AXL_SE_D16::readFromModule()
     */
    bool update();
};


#endif //H2ZMU_AXCF_AXL_SE_DI16_H
