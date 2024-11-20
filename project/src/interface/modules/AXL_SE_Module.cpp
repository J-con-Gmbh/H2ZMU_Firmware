//
// Created by jriessner on 18.12.23.
//

#include <grpcpp/create_channel.h>
#include "interface/modules/AXL_SE_Module.h"

AXL_SE_Module::AXL_SE_Module(std::string port) {
    this->_port = std::move(port);

    this->channel = grpc::CreateChannel(
            "unix:/run/plcnext/grpc.sock",
            grpc::InsecureChannelCredentials());

    this->dataAccessService = Arp::Plc::Gds::Services::Grpc::IDataAccessService();
    this->dataAccessStub = Arp::Plc::Gds::Services::Grpc::IDataAccessService::NewStub(channel);

}

std::tuple<bool, uint16_t> AXL_SE_Module::writeSingleWord(uint16_t controlWord) {

    union {
        uint16_t word;
        uint8_t bytes[2];
    };
    word = controlWord;

    return writeByteVector({bytes[0], bytes[1]});
}

std::tuple<bool, uint16_t> AXL_SE_Module::writeByteVector(const std::vector<uint8_t>& data) {

    grpc::ClientContext ctx;
    Arp::Plc::Gds::Services::Grpc::IDataAccessServiceWriteResponse response;
    Arp::Plc::Gds::Services::Grpc::IDataAccessServiceWriteRequest request;

    auto item = request.add_data();
    item->set_portname(this->_port);
    item->mutable_value()->set_typecode(Arp::Type::Grpc::CoreType::CT_Array);

    for (const auto &byte: data) {

        Arp::Type::Grpc::ObjectType *arrElmt1 = item->mutable_value()->mutable_arrayvalue()->add_arrayelements();
        arrElmt1->set_typecode(::Arp::Type::Grpc::CoreType::CT_Char);
        arrElmt1->set_charvalue(byte);

    }

    auto status = this->dataAccessStub->Write(&ctx, request, &response);

    if (!status.ok()) {
        /// TODO return status bits
        return {false, 0};
    }

    /// TODO return status bits
    return {true, 0};
}

std::tuple<bool, std::vector<uint8_t>> AXL_SE_Module::readByteVector() {

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

    std::memcpy(&this->sendStatus.last, &return_array[0], sizeof(return_array[0]));

    return {true, return_array};
}

std::tuple<bool, std::vector<uint8_t>> AXL_SE_Module::writeCmdOk(uint8_t cmd)  {

    std::tuple<bool, std::vector<uint8_t>> handshake;
    handshake = this->readByteVector();
    auto vector = std::get<std::vector<uint8_t>>(handshake);

    //std::cout << 0x01 << std::endl;

    if (        std::get<bool>(handshake)
                && !vector.empty()
                &&  vector[0] == cmd) {

        return {true, vector};
    }
    return {false, vector};
}

bool AXL_SE_Module::Tx_buf_not_empty() const {
    return this->sendStatus.last.tx_buf_not_empty;
}

bool AXL_SE_Module::Rx_buf_not_empty() const {
    return this->sendStatus.last.rx_buf_not_empty;
}
