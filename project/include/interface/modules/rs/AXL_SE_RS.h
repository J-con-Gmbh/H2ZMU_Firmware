//
// Created by jriessner on 17.12.23.
//

#ifndef H2ZMU_AXCF_AXL_SE_RS_H
#define H2ZMU_AXCF_AXL_SE_RS_H


#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>
#include <string>
#include <grpcpp/channel.h>
#include <bitset>
#include "Plc/Gds/IDataAccessService.grpc.pb.h"
#include "interface/modules/AXL_SE_Module.h"

class AXL_SE_RS {
    AXL_SE_Module input;
    AXL_SE_Module output;


public:
    static bool decrTillZero(int &counter);

    static const uint8_t ACTION_SEND;
    static const uint8_t ACTION_READ;

    static const uint8_t CMD_READ_REC_BUFFER_STATE;
    static const uint8_t CMD_SEND_DATA;
    static const uint8_t CMD_CACHE_DATA_TO_SEND;
    static const uint8_t CMD_READ_CHARS_OR_COUNTER;

    static const uint8_t CMD_TOGGLE_SEND_DATA;
    static const uint8_t CMD_TOGGLE_CACHE_DATA_TO_SEND;
    static const uint8_t CMD_TOGGLE_READ_DATA;

    static const uint8_t CMD_READ_REC_BUFFER_COUNTER;
    static const uint8_t CMD_READ_DATA;
    static const uint8_t CMD_READ_COUNTER;
    static const uint8_t CMD_SAVE_FOR_SEND;
    static const uint8_t CMD_SEND_DATA_17;

    static const uint8_t STS_TX_BUF_NOT_EMPTY;
    static const uint8_t STS_TX_BUF_FULL;
    static const uint8_t STS_RX_BUF_FULL;
    static const uint8_t STS_RX_BUF_NOT_EMPTY;


    explicit AXL_SE_RS(std::string port) : output(std::move(port)), input(std::move(port)){};
    AXL_SE_RS(std::string input, std::string output) : input(std::move(input)) , output(std::move(output)){};

    std::tuple<bool, uint16_t> sendPayload(uint8_t one, uint8_t two, uint8_t three, std::vector<uint8_t> data);
    std::tuple<bool, uint16_t> sendData(std::vector<uint8_t> data, bool validate = false);
    bool writeCmdOk(uint8_t cmd, uint8_t action);

    bool dataPending();

    std::tuple<bool, uint8_t, std::vector<uint8_t>> receiveData();

    bool cacheData(std::vector<uint8_t> data, bool toggle);
};


#endif //H2ZMU_AXCF_AXL_SE_RS_H
