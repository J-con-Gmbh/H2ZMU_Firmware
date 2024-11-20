//
// Created by afortino on 09.02.24.
//

#include "interface/modules/hart/AXL_HART.h"

#include "Plc/Gds/IDataAccessService.grpc.pb.h"
#include <grpcpp/security/credentials.h>
#include <bitset>
#include <thread>
#include <numeric>

const uint8_t AXL_HART::DEFAULT_HART_VALUE = 0x7F;
const uint8_t AXL_HART::MAX_HART_PARTICIPANTS = 8;

float AXL_HART::uint8ToFloat(const uint8_t bytes[4]) {
    static_assert(sizeof(float) == 4, "Float size is not 4 bytes, cannot convert");
    
    float result;
    std::memcpy(&result, bytes, sizeof(float));
    return result;
}

// 80 04 = Messwert ungültig oder kein gültiger Messwert verfügbar
std::tuple<bool, uint8_t, std::vector<float>> AXL_HART::receiveData() {
    //std::cout << "Start Recieve Data" << std::endl;
    
    std::vector<float> return_array;
    std::vector<hart_participant> sensor_values;
    auto response = Arp::Plc::Gds::Services::Grpc::IDataAccessServiceReadResponse();

    // Read amount of data available
    auto buffer = this->output.readByteVector();
    if (!std::get<bool>(buffer)) {
        return {false, 0, {}};
    }

    std::vector<uint8_t> buffer_values = std::get<std::vector<uint8_t>>(buffer);

    for (int i = 0; i < buffer_values.size() / MAX_HART_PARTICIPANTS; i++) {
        struct hart_participant value;
        value.status1 = buffer_values[i * 2 + 0];
        value.status2 = buffer_values[i * 2 + 1];
        value.value1 = buffer_values[i * 4 + 16];
        value.value2 = buffer_values[i * 4 + 17];
        value.value3 = buffer_values[i * 4 + 18];
        value.value4 = buffer_values[i * 4 + 19];

        if(value.value1 != DEFAULT_HART_VALUE) {
            sensor_values.emplace_back(value);
        }
    }

    for (const auto &item: sensor_values) {
        uint8_t bytes[] = { item.value4, item.value3, item.value2, item.value1 };
        float floatValue = this->uint8ToFloat(bytes);
        return_array.emplace_back(floatValue);
    }

    return {true, 0, return_array};
}


