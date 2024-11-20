//
// Created by jriessner on 09.02.24.
//

#ifndef TEST_MB_STRUCTS_H
#define TEST_MB_STRUCTS_H

#include <bits/stdint-uintn.h>
#include "mb_definitions.h"

struct mb_ctx {
    // modbus server id
    uint8_t s_id;

    uint8_t count_do;
    uint8_t count_di;
    uint8_t count_ao;
    uint8_t count_ai;

    uint8_t *reg_do;
    uint8_t *reg_di;
    uint16_t *reg_ao;
    uint16_t *reg_ai;

};

struct mb_telegram {
    uint8_t *payload;
    // mb server id
    uint8_t *s_id;
    // invoked modbus function
    uint8_t *mb_fn_code;
    uint16_t *first_word;
    uint16_t *second_word;
    uint16_t *first_byte;
    uint16_t *second_byte;


    uint16_t *crc;

    uint8_t size;
};

enum MB_FN {
    READ_DO = MB_FN_01,
    READ_DI = MB_FN_02,
    READ_AO = MB_FN_03,
    READ_AI = MB_FN_04,
    WRITE_DO = MB_FN_05,
    WRITE_AO = MB_FN_06,
    WRITE_MULTIPLE_DO = MB_FN_15,
    WRITE_MULTIPLE_AO = MB_FN_16
};

enum  MB_EXC {
    ILLEGAL_FUNCTION = MB_EXC_01,
    ILLEGAL_DATA_ACCESS = MB_EXC_02,
    ILLEGAL_DATA_VALUE = MB_EXC_03,
    SLAVE_DEVICE_FAILURE = MB_EXC_04,
    ACKNOWLEDGE = MB_EXC_05,
    SLAVE_DEVICE_BUSY = MB_EXC_06,
    NEGATIVE_ACKNOWLEDGEMENT = MB_EXC_07,
    MEMORY_PARITY_ERROR = MB_EXC_08,
    GATEWAY_PATH_UNAVAILABLE = MB_EXC_10,
    GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND = MB_EXC_11
};


#endif //TEST_MB_STRUCTS_H
