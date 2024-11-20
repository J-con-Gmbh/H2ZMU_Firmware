//
// Created by jriessner on 18.12.23.
//

#ifndef H2ZMU_AXCF_AXL_SE_D16_H
#define H2ZMU_AXCF_AXL_SE_D16_H

#include <utility>
#include <bitset>

#include "interface/modules/AXL_SE_Module.h"

class AXL_SE_D16 : public AXL_SE_Module {
protected:
    enum BIT {
        _0 = 0b00000001,
        _1 = 0b00000010,
        _2 = 0b00000100,
        _3 = 0b00001000,
        _4 = 0b00010000,
        _5 = 0b00100000,
        _6 = 0b01000000,
        _7 = 0b10000000
    };

    std::bitset<16> currentState{0};
    bool _onHold = false;

    /**
     * This function reads the current State at the digital Inputs
     * @return success of the gRPC Call
     */
    std::tuple<bool, uint16_t, std::vector<uint16_t>> receiveData();

    /**
     * This functions Calls AXL_SE_D16::receiveData(), interprets the received data, and stores it in the internal cache-variable this->currentState
     * @return (indirect) success of AXL_SE_D16::receiveData()
     */
    bool readFromModule();

public:
    explicit AXL_SE_D16(const std::string& port): AXL_SE_Module(port) {};

    bool update();
    void setOnHold(bool onHold);
    std::tuple<bool, bool> getStateOfBit(uint8_t bit);
    std::bitset<16> getCurrentState();
};


#endif //H2ZMU_AXCF_AXL_SE_D16_H
