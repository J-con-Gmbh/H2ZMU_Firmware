//
// Created by afortino on 09.02.24.
//

#ifndef H2ZMU_AXCF_AXL_HART_MODULE_H
#define H2ZMU_AXCF_AXL_HART_MODULE_H

#define MAX_IT(c) (0 < (c--))

#include <string>
#include "Plc/Gds/IDataAccessService.grpc.pb.h"

struct hart_participant {
    uint8_t status1;
    uint8_t status2;
    uint8_t value1;
    uint8_t value2;
    uint8_t value3;
    uint8_t value4;
};

class AXL_HART_Module {
protected:
    std::string _port;
    std::shared_ptr<grpc::Channel> channel;
    Arp::Plc::Gds::Services::Grpc::IDataAccessService dataAccessService;

public:
    std::unique_ptr<Arp::Plc::Gds::Services::Grpc::IDataAccessService::Stub> dataAccessStub;
    std::vector<hart_participant> sensor_values;
protected:

public:

    explicit AXL_HART_Module(std::string port);
    std::tuple<bool, std::vector<uint8_t>> readByteVector();
};


#endif //H2ZMU_AXCF_AXL_HART_MODULE_H
