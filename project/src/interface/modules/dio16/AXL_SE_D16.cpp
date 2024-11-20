//
// Created by jriessner on 18.12.23.
//

#include <bitset>
#include "interface/modules/dio16/AXL_SE_D16.h"
#include "data/db/repositories/OccurredStatusRepository.h"

std::tuple<bool, bool> AXL_SE_D16::getStateOfBit(uint8_t bit) {
    if (!this->_onHold && !this->readFromModule()) {
        return {false, false};
    }
    return {true, this->currentState[bit]};
}

std::tuple<bool, uint16_t, std::vector<uint16_t>> AXL_SE_D16::receiveData() {

    auto ctx = grpc::ClientContext();
    auto request = Arp::Plc::Gds::Services::Grpc::IDataAccessServiceReadRequest();
    request.add_portnames(this->_port);
    auto response = Arp::Plc::Gds::Services::Grpc::IDataAccessServiceReadResponse();

    auto status = this->dataAccessStub->Read(&ctx, request, &response);
    static uint16_t last_value = 0;

    if (!status.ok()) {
        /// TODO return status bits
        return {false, 0, {}};
    }

    auto returned_value = response._returnvalue();
    auto return_vector = std::vector<uint16_t>();
    for (const auto &item: returned_value) {
        uint16_t value = item.value().uint16value();
        return_vector.emplace_back(value);
    }

    if(last_value != 0 && return_vector[0] != last_value)
    {
        struct occurredstatus _status;
        _status.digitalIn = return_vector[0];
        OccurredStatusRepository::instance->persistOccurredStatus(&_status);
    }   
    
    /// TODO return status bits
    return {true, 0, return_vector};

}

bool AXL_SE_D16::readFromModule() {
    auto tuple = this->receiveData();
    if (!std::get<bool>(tuple)) {
        return false;
    }
    uint16_t word = std::get<std::vector<uint16_t>>(tuple)[0];
    currentState = std::bitset<16>(word);

    return true;
}

void AXL_SE_D16::setOnHold(bool onHold) {
    this->_onHold = onHold;
}

std::bitset<16> AXL_SE_D16::getCurrentState() {
    return this->currentState;
}

bool AXL_SE_D16::update() {
    return this->readFromModule();
}
