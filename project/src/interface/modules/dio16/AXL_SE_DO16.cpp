//
// Created by jriessner on 18.12.23.
//

#include "interface/modules/dio16/AXL_SE_DO16.h"
#include "data/db/repositories/OccurredStatusRepository.h"

#include <utility>

AXL_SE_DO16::AXL_SE_DO16(std::string port) : AXL_SE_D16(std::move(port)) {
    this->readFromModule();
}

bool AXL_SE_DO16::setStateOfBit(uint8_t bit, bool state) {
    this->currentState.set(bit, state);
    std::cout << "Bit " << unsigned(bit) << " was set to " << state << std::endl;
    if (!this->_onHold) {
        return this->writeToModule();
    }

    return true;
}

bool AXL_SE_DO16::writeToModule() {

    auto ctx = grpc::ClientContext();
    auto request = Arp::Plc::Gds::Services::Grpc::IDataAccessServiceWriteRequest();
    // See https://www.plcnext.help/te/Service_Components/Remote_Service_Calls_RSC/RSC_GDS_services.htm#IDataAccessService
    auto item = request.add_data();
    item->set_portname(this->_port);
    item->mutable_value()->set_uint16value((uint16_t) this->currentState.to_ulong());
    item->mutable_value()->set_typecode(Arp::Type::Grpc::CoreType::CT_Uint16);
    static uint16_t last_value = 0;
    uint16_t value = (uint16_t) this->currentState.to_ulong();

    if(last_value != 0 && value != last_value)
    {
        struct occurredstatus _status;
        _status.digitalOut = value;
        OccurredStatusRepository::instance->persistOccurredStatus(&_status);
    }   

    auto response = Arp::Plc::Gds::Services::Grpc::IDataAccessServiceWriteResponse();

    auto status = this->dataAccessStub->Write(&ctx, request, &response);

    //std::cout << this->activePort << " written " << this->currentState << " to module" << std::endl;

    return status.ok();
}

bool AXL_SE_DO16::flush() {
    return this->writeToModule();
}

bool AXL_SE_DO16::unsetAll() {

    this->currentState = std::bitset<16>(0);

    if (!this->_onHold)
        return this->writeToModule();

    return true;
}
