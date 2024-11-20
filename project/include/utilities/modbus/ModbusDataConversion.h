//
// Created by jriessner on 07.09.23.
//

#ifndef H2ZMU_V1_MODBUSDATACONVERSION_H
#define H2ZMU_V1_MODBUSDATACONVERSION_H

#include <vector>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <sstream>
#include "data/db/entities/e_Param.h"
#include "datatypes.h"
#include "modbus_interface.h"


/**
 * This function converts modbus register-data to the given simple type.<br>
 * It is necessary, that the data is in big-endian format (high byte first, high word first)
 * @tparam T template type
 * @param data data to convert
 * @return value as type T
 */
template < typename T >
T registerDataToSimpleType(const std::vector<uint16_t>& data) {

    size_t typeSize = sizeof(T);
    size_t dataSize = (typeSize / 2) + (typeSize % 2);
    union uTemplate{
        uint8_t bytes[((sizeof(T) / 2) + (sizeof(T) % 2)) * 2];
        uint16_t split[((sizeof(T) / 2) + (sizeof(T) % 2))];
        T value;
    };
    uTemplate ret{0};

    if ((data.size()*2) != typeSize) {
        return (T) 0;
    }
    for (int i = 0; (i < dataSize); ++i) {
        ret.split[i] = data[i];
    }

    return ret.value;
}

/**
 * This function converts modbus register-data to the given simple type.<br>
 * It is necessary, that the data is in big-endian format (high byte first, high word first)
 * @tparam T template type
 * @param data data to convert
 * @return value as type T
 */
template < typename T >
std::vector<u_int16_t> simpleTypeToRegister(T value) {

    size_t typeSize = sizeof(T);
    size_t dataSize = (typeSize / 2) + (typeSize % 2);
    union uTemplate{
        uint8_t bytes[((sizeof(T) / 2) + (sizeof(T) % 2)) * 2];
        uint16_t split[((sizeof(T) / 2) + (sizeof(T) % 2))];
        T value;
    };
    uTemplate temp{.value=value};
    std::vector<u_int16_t> ret;

    for (int i = 0; i < dataSize; ++i) {
        ret.template emplace_back(temp.split[i]);
    }

    return ret;
}

/**
 * This function converts a struct param to the given simple type.<br>
 * It is necessary, that the data is in big-endian format (high byte first, high word first)
 * @tparam T template type
 * @param data data to convert
 * @return value as type T
 */
template < typename T >
T stringToSimpleType(const std::string& value) {
    T ret;
    std::stringstream convert(value);
    convert >> ret;

    return ret;
}

/**
 * This function converts modbus register-data to the given simple type.<br>
 * It is necessary, that the data is in big-endian format (high word first, high byte first) <br>
 * Hex values can be checked on the following websites: <br>
 * - https://www.rapidtables.com/convert/number/hex-to-ascii.html <br>
 * - https://gregstoll.com/~gregstoll/floattohex/ <br>
 * @tparam T template type
 * @param data datatype to convert
 * @return value as register data
 */
template < typename T >
std::vector<uint16_t> dataTypeToRegisterData(T data) {
    std::vector<uint16_t> ret;
    union {
        uint16_t split[((sizeof(T) / 2) + (sizeof(T) % 2))];
        T value;
    };
    value = data;
    std::copy(&split[0], &split[(sizeof(T)/2 + (sizeof(T) % 2))], back_inserter(ret));
    std::reverse(ret.begin(), ret.end());

    return ret;
}

/**
 * This function converts modbus register data to a std::string.<br>
 * It is necessary, that the data is in big-endian format (high byte first, high word first)
 * @param data data to convert
 * @return value std::string
 */
std::string registerDataToString(const std::vector<uint16_t>& data);

/**
 * This function converts modbus register data to a std::string.<br>
 * It is necessary, that the data is in big-endian format (high byte first, high word first)
 * @param data data to convert
 * @return value std::string
 */
std::string registerDataToStringBigEndian(const std::vector<uint16_t>& data);

/**
 * This function converts a std::string to modbus register data.<br>
 * It is necessary, that the data is in big-endian format (high byte first, high word first)
 * @param std::string to convert
 * @return register data
 */
std::vector<uint16_t> stringToRegisterData(const std::string& value, int maxLen = 20, mb::endianness ends = {true, true});

/**
 * This function converts a parameter struct to modbus data<br>
 * The data is big-endian format (high byte first, high word first)<br>
 * Type conversion is handled by function <b>dataTypeToRegister(T data)</b>
 * @param parameter param to convert
 * @return modbus data
 */
std::vector<uint16_t> parameterToRegisterData(const struct param& parameter);

/**
 * This function converts modbus data to a parameter value, and returns a copy of the given parameter with the updated value <br>
 * The data has to be in big-endian format (high byte first, high word first)<br>
 * Type conversion is handled by function <b>dataTypeToRegister(T data)</b>
 * @param parameter param to convert
 */
struct param registerDataToParameter(const std::vector<uint16_t>& data, struct param parameter);

#endif //H2ZMU_V1_MODBUSDATACONVERSION_H
