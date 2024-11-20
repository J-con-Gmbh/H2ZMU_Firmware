//
// Created by afortino on 09.02.24.
//

#ifndef H2ZMU_AXCF_AXL_HART_H
#define H2ZMU_AXCF_AXL_HART_H


#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>
#include <string>
#include <grpcpp/channel.h>
#include <bitset>
#include "Plc/Gds/IDataAccessService.grpc.pb.h"
#include "interface/modules/AXL_HART_Module.h"

class AXL_HART {
    AXL_HART_Module output;


public:

    static const uint8_t DEFAULT_HART_VALUE;
    static const uint8_t MAX_HART_PARTICIPANTS;
    //explicit AXL_HART(std::string port) : output(std::move(port)){};
    AXL_HART(std::string output) :output(std::move(output)){};

    std::tuple<bool, uint8_t, std::vector<float>> receiveData();
    float uint8ToFloat(const uint8_t bytes[4]);
};


#endif //H2ZMU_AXCF_AXL_HART_H
