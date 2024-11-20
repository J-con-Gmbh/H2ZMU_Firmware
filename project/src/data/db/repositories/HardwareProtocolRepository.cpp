//
// Created by jriessner on 19.06.2022.
//

#include "data/db/repositories/HardwareProtocolRepository.h"

std::shared_ptr<HardwareProtocolRepository> HardwareProtocolRepository::instance;

bool HardwareProtocolRepository::loadAll() {
    std::string sql = "SELECT * FROM hardwareprotocols;";
    std::string ret = databaseService->executeSqlReturn(sql);

    size_t pos = 0;
    std::string token;
    while ((pos = ret.find(';')) != std::string::npos) {
        token = ret.substr(0, pos);
        struct hardwareprotocol _hardwareprotocol = getProtocolFromQuery(token);
        allProtocols[_hardwareprotocol.id] = _hardwareprotocol;
        ret.erase(0, pos + 1);
    }

    return false;
}

struct hardwareprotocol HardwareProtocolRepository::getProtocolById(int id) {
    if (allProtocols.count(id)) {
        return allProtocols[id];
    }

    return {};
}

struct hardwareprotocol HardwareProtocolRepository::getProtocolFromQuery(const std::string &query) {
    struct hardwareprotocol _protocol;
    _protocol.id = std::stoi(utils::strings::getValueFromQuery(query, "id", ","));
    _protocol.name = utils::strings::getValueFromQuery(query, "name", ",");
    _protocol.rawValueMax = std::stof(utils::strings::getValueFromQuery(query, "rawvaluemax", ","));
    _protocol.rawValueMin = std::stof(utils::strings::getValueFromQuery(query, "rawvaluemin", ","));

    return _protocol;
}

std::map<int, struct hardwareprotocol> HardwareProtocolRepository::getAllProtocols() {
    return allProtocols;
}
