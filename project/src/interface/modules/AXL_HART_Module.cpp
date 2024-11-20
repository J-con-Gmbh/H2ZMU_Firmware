//
// Created by afortino on 09.02.24.
//

#include <grpcpp/create_channel.h>
#include "interface/modules/AXL_HART_Module.h"

AXL_HART_Module::AXL_HART_Module(std::string port) {
    this->_port = std::move(port);

    this->channel = grpc::CreateChannel(
            "unix:/run/plcnext/grpc.sock",
            grpc::InsecureChannelCredentials());

    this->dataAccessService = Arp::Plc::Gds::Services::Grpc::IDataAccessService();
    this->dataAccessStub = Arp::Plc::Gds::Services::Grpc::IDataAccessService::NewStub(channel);

}

std::tuple<bool, std::vector<uint8_t>> AXL_HART_Module::readByteVector() {

    auto ctx = grpc::ClientContext();
    auto request = Arp::Plc::Gds::Services::Grpc::IDataAccessServiceReadRequest();
    request.add_portnames(this->_port);
    auto response = Arp::Plc::Gds::Services::Grpc::IDataAccessServiceReadResponse();

    auto status = this->dataAccessStub->Read(&ctx, request, &response);

    if (!status.ok()) {
        return {false, {}};
    }

    std::vector<uint8_t> return_array;
    auto response_value = response._returnvalue();
    for (const auto &item: response_value) {
        for (const auto & it : item.value().arrayvalue().arrayelements()) {
            return_array.emplace_back(it.uint8value());
        }
    }

    return {true, return_array};
}