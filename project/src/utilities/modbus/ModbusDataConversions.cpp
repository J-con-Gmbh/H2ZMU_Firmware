//
// Created by jriessner on 07.09.23.
//

#include "utilities/modbus/ModbusDataConversion.h"


std::string registerDataToString(const std::vector<uint16_t>& data) {
    std::string ret;
    union word_byte {
        uint16_t value = 0;
        char split[2];
    };
    for (const auto &item: data) {
        word_byte wb;
        wb.value = item;
        if (wb.split[0] != 0x03) {
            ret += wb.split[0];
        } else { break;}

        if (wb.split[1] != 0x03) {
            ret += wb.split[1];
        } else { break;}
    }

    return ret;
}

std::string registerDataToStringBigEndian(const std::vector<uint16_t>& data) {
    std::string ret;
    
    for (const auto &item: data) {
        // Extract the two bytes from the uint16_t
        char firstByte = static_cast<char>((item >> 8) & 0xFF);
        char secondByte = static_cast<char>(item & 0xFF);

        // Check if the first byte is not equal to 0x03
        if (firstByte != 0x03) {
            ret += firstByte;
        } else {
            break;
        }

        // Check if the second byte is not equal to 0x03
        if (secondByte != 0x03) {
            ret += secondByte;
        } else {
            break;
        }
    }
    
    return ret;
}

std::vector<uint16_t> stringToRegisterData(const std::string& value, int maxLen, mb::endianness ends) {

    std::vector<uint16_t> ret( (value.size() / 2) + 1, 0x0000);
    int v_size = value.size();

    for (int i = 0; i < v_size; ++i) {

        char c = value.at(i);
        if (i % 2 == 0) {
            ret[i / 2] = ret[i / 2] | c;
        } else {
            ret[i / 2] = ret[i / 2] | (c << 8);
        }
    }

    if (v_size <= maxLen) {
        if (v_size % 2 == 0) {
            ret[v_size / 2] = ret[v_size / 2] | 0x03;
        } else {
            ret[v_size / 2] = ret[v_size / 2] | (0x03 << 8);
        }
    }

    ret = mb::from_to_endian(ret, {true, true}, ends);

    return ret;
}

std::vector<uint16_t> parameterToRegisterData(const struct param& parameter) {
    std::vector<uint16_t> ret;
    switch ((datatype) parameter.datatype) {

        case INT:
            ret = dataTypeToRegisterData<int>(stringToSimpleType<int>(parameter.value));
            break;
        case LONG:
            ret = dataTypeToRegisterData<long>(stringToSimpleType<long>(parameter.value));
            break;
        case BOOL:
            ret = dataTypeToRegisterData<bool>(stringToSimpleType<bool>(parameter.value));
            break;
        case FLOAT:
            ret = dataTypeToRegisterData<float>(stringToSimpleType<float>(parameter.value));
            break;
        case CHAR:
            ret = dataTypeToRegisterData<char>(stringToSimpleType<char>(parameter.value));
            break;
        case STRING:
            ret = mb::ModbusServer::stringToRegisterData(parameter.value);
            break;
    }

    return ret;
}

struct param registerDataToParameter(const std::vector<uint16_t>& data, struct param parameter) {
    switch ((datatype) parameter.datatype) {

        case INT: {
            parameter.value = std::to_string(registerDataToSimpleType<int>(data));
        } break;
        case LONG: {
            parameter.value = std::to_string(registerDataToSimpleType<long>(data));
        } break;
        case BOOL: {
            parameter.value = std::to_string(registerDataToSimpleType<bool>(data));
        } break;
        case FLOAT: {
            parameter.value = std::to_string(registerDataToSimpleType<float>(data));
        } break;
        case CHAR: {
            parameter.value = std::to_string(registerDataToSimpleType<char>(data));
        } break;
        case STRING: {
            parameter.value = registerDataToString(data);
        } break;
    }

    return parameter;
}