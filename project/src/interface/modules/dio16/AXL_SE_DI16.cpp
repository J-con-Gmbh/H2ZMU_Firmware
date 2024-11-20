//
// Created by jriessner on 18.12.23.
//

#include "interface/modules/dio16/AXL_SE_DI16.h"

bool AXL_SE_DI16::update() {
    return this->readFromModule();
}
