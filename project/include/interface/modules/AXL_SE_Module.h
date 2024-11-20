//
// Created by jriessner on 18.12.23.
//

#ifndef H2ZMU_AXCF_AXL_SE_MODULE_H
#define H2ZMU_AXCF_AXL_SE_MODULE_H

#define MAX_IT(c) (0 < (c--))

#include <string>
#include "Plc/Gds/IDataAccessService.grpc.pb.h"

struct send_status_bits {
    unsigned    reserved1:1,
                tx_buf_not_empty:1,
                tx_buf_full:1,
                rx_buf_full:1,
                reserved2:3,
                rx_buf_not_empty:1;
};

union send_status {
    uint8_t byte;
    send_status_bits last;
};

class AXL_SE_Module {
protected:
    std::string _port;
    std::shared_ptr<grpc::Channel> channel;
    Arp::Plc::Gds::Services::Grpc::IDataAccessService dataAccessService;

    send_status sendStatus;


public:
    std::unique_ptr<Arp::Plc::Gds::Services::Grpc::IDataAccessService::Stub> dataAccessStub;
protected:

    /**
     * This function tests the "Status word low byte" for the state of the specified bit <br>
     * <a href="https://pxc-crisp-production-platform-cr-s3downloadbucket-1rf23da6xdlmt.s3.eu-west-1.amazonaws.com/7936733?response-content-disposition=inline%3B%20filename%3D%22db_de_axl_se_rs485_108701_de_04.pdf%22&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Date=20231217T181915Z&X-Amz-SignedHeaders=host&X-Amz-Expires=10800&X-Amz-Credential=AKIAWWXX4BDRFPQUJUER%2F20231217%2Feu-west-1%2Fs3%2Faws4_request&X-Amz-Signature=15f28fcc72f1ec57b412144e0d6bfcb303832e260d195c4c952bf40b5e9d2c8f#G1028767">
     Datasheet</a>
     * @param statusByte
     * @param bitToCheck
     * @return true if bit is high
     */
    static bool checkForStatusBit(uint8_t statusByte, uint8_t bitToCheck);

public:

    explicit AXL_SE_Module(std::string port);
    std::tuple<bool, uint16_t> writeSingleWord(uint16_t controlWord);

    std::tuple<bool, uint16_t> writeByteVector(const std::vector<uint8_t>& data);
    std::tuple<bool, std::vector<uint8_t>> readByteVector();

    std::tuple<bool, std::vector<uint8_t>> writeCmdOk(uint8_t cmd);

    bool Tx_buf_not_empty() const;
    bool Rx_buf_not_empty() const;
};


#endif //H2ZMU_AXCF_AXL_SE_MODULE_H
