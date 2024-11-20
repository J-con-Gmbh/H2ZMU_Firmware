//
// Created by jriessner on 22.08.2022.
//

#ifndef H2ZMU_2_HARTINTERFACE_H
#define H2ZMU_2_HARTINTERFACE_H


#include <cstdint>
#include <memory>
#include <vector>
#include "modbus_interface.h"

class HartInterface {
private:
    static std::shared_ptr<mb::ModbusClient> modbusClient;
    static int offset;
    static int setRegLength;

public:
    static void init(std::shared_ptr<mb::ModbusClient> client);
    static std::vector<uint16_t> getDataFromDevice(int devId);
    static float mapDataToFloat(uint16_t reg1, uint16_t reg2);
};


#endif //H2ZMU_2_HARTINTERFACE_H
