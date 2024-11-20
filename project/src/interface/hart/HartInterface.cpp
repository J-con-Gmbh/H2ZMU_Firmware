//
// Created by jriessner on 22.08.2022.
//

#include "interface/hart/HartInterface.h"

#include <utility>


std::shared_ptr<mb::ModbusClient> HartInterface::modbusClient;
int HartInterface::offset;
int HartInterface::setRegLength;

std::vector<uint16_t> HartInterface::getDataFromDevice(int devId) {

    std::vector<uint16_t> ret;
    int mbRegister = HartInterface::offset + ((devId) * HartInterface::setRegLength);
    ret = HartInterface::modbusClient->readInputRegisters(mbRegister, HartInterface::setRegLength);

    return ret;
}

void HartInterface::init(std::shared_ptr<mb::ModbusClient> client) {
    //TODO Offset Parametrisieren
    HartInterface::modbusClient = std::move(client);
    HartInterface::offset = 1300;
    HartInterface::setRegLength = 10;
}

float HartInterface::mapDataToFloat(uint16_t reg1, uint16_t reg2) {
    float val = mb::ModbusClient::readFloatFromRegister(reg1, reg2);

    return val;
}
