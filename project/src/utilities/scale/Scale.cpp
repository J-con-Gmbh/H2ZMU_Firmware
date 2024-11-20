//
// Created by jriessner on 18.08.23.
//

#include "utilities/scale/Scale.h"
#include "Core.h"

bool Scale::setup(const std::string& fd, bool debug) {
    return modbusClient.setup(fd, debug);
}

Scale::Scale() {
    if (!Core::instance->config->isTrue("SCALE_ACTIVE")) {
        return;
    }
    setup(Core::instance->config->getValue("SCALE_FD"));
}

float Scale::fetchData() {
    if (! this->modbusClient.isActive() ) {
        return -1;
    }

    scale_state state;

    std::vector<uint16_t> default_data = this->modbusClient.readHoldingRegisters(DEFAULT, 2);
    std::vector<uint16_t> gross_data = this->modbusClient.readHoldingRegisters(GROSS, 2);
    std::vector<uint16_t> tare_data = this->modbusClient.readHoldingRegisters(TARE, 2);
    std::vector<uint16_t> net_data = this->modbusClient.readHoldingRegisters(NET, 2);
    std::vector<uint16_t> raw_data = this->modbusClient.readHoldingRegisters(RAW, 2);

    state.default_value = mb::ModbusClient::readFloatFromRegister(default_data[0],  default_data[1],    false, true, true);
    state.gross_value   = mb::ModbusClient::readFloatFromRegister(gross_data[0],    gross_data[1],      false, true, true);
    state.tare_value    = mb::ModbusClient::readFloatFromRegister(tare_data[0],     tare_data[1],       false, true, true);
    state.net_value     = mb::ModbusClient::readFloatFromRegister(net_data[0],      net_data[1],        false, true, true);
    state.raw_value     = mb::ModbusClient::readFloatFromRegister(raw_data[0],      raw_data[1],        false, true, true);

    //this->queue.add(state);

    return state.default_value;
}
