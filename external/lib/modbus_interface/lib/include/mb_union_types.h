//
// Created by jriessner on 27.02.23.
//

#ifndef MODBUS_INTERFACE_MB_UNION_TYPES_H
#define MODBUS_INTERFACE_MB_UNION_TYPES_H

#include <cstdint>

namespace mb {
    union uint16_8 {
        uint16_t value;
        uint8_t split[2];
    };

    union reg_int64 {
        int64_t value;
        uint16_t data[4];
    };

    union reg_int32 {
        int value;
        uint16_t data[2];
    };

    union reg_float32 {
        float value;
        uint16_t data[2];
    };

    union float32_int8 {
        float value;
        uint8_t split[4];
    };

    union reg_bool {
        bool value;
        uint16_t data[1];
    };

    union reg_unsigned_chars {
        unsigned char value[2];
        uint16_t data = 0x0000;
    };
}
#endif //MODBUS_INTERFACE_MB_UNION_TYPES_H
