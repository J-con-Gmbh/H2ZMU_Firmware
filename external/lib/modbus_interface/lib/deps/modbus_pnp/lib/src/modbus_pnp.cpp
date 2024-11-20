//
// Created by jriessner on 09.02.24.
//

#include <stdio.h>
#include <bits/stdint-uintn.h>
#include <malloc.h>
#include "../include/modbus_pnp.h"

uint8_t reverse_byte (uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

uint16_t switch_bytes(uint16_t w) {
    return (w & 0x00FF) << 8 | (w & 0xFF00) >> 8;
}

uint16_t calculate_crc(const struct mb_telegram* telegram) {
    uint16_t crc = 0xFFFF;

    for (int pos = 0; pos < telegram->size - 2; pos++) {
        crc ^= (uint16_t)telegram->payload[pos];          // XOR byte into least sig. byte of crc
        for (int i = 0; i < 8; ++i) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = (crc >> 1);
        }
    }
    printf("\n");

    // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
    return crc;
}


struct mb_ctx init_context(uint8_t s_id, uint8_t count_do, uint8_t count_di, uint8_t count_ao, uint8_t count_ai) {
    struct mb_ctx ctx;
    ctx.s_id = s_id;

    ctx.count_do = count_do;
    ctx.count_di = count_di;
    ctx.count_ao = count_ao;
    ctx.count_ai = count_ai;

    if (count_do > 0) ctx.reg_do = (uint8_t*) malloc(count_do);
    if (count_di > 0) ctx.reg_di = (uint8_t*) malloc(count_di);

    if (count_ao > 0) ctx.reg_ao = (uint16_t*) malloc(count_ao*2);
    if (count_ai > 0) ctx.reg_ai = (uint16_t*) malloc(count_ai*2);

    return ctx;
}

void free_context(struct mb_ctx ctx) {
    free(ctx.reg_do);
    free(ctx.reg_di);
    free(ctx.reg_ao);
    free(ctx.reg_ai);
}

struct mb_telegram init_telegram() {
    return init_telegram_data(0);
}

struct mb_telegram init_telegram_data(uint8_t data_len) {
    return init_telegram_data_ctrl_bytes(data_len, 8);
}

struct mb_telegram init_telegram_data_ctrl_bytes(uint8_t data_len, uint8_t count_ctl_bytes) {
    struct mb_telegram telegram;
    telegram.size = (uint8_t) (data_len + count_ctl_bytes);

    telegram.payload = (uint8_t *)(malloc(telegram.size));

    telegram.s_id = telegram.payload;
    telegram.mb_fn_code = (telegram.payload + 1);

    telegram.first_word = (uint16_t *) (telegram.payload + 2);
    telegram.second_word = (uint16_t *) (telegram.payload + 4);

    telegram.crc = (uint16_t *) (telegram.payload + telegram.size - 2);

    return telegram;
}

struct mb_telegram parse_telegram(const uint8_t* data, uint8_t data_len) {
    struct mb_telegram telegram = init_telegram_data(data_len - 8);
    for (int i = 0; i < data_len; ++i) {
        telegram.payload[i] = data[i];
        printf("0x%02x ", (uint8_t) (data[i] & 0xff));
    }
    printf("\n");

    uint16_t crc = calculate_crc(&telegram);
    if (crc != *telegram.crc) {
        fprintf(stderr, "Error: Transmitted CRC [0x%04x] not matching calculatet CRC [0x%04x]", crc, *telegram.crc);
    }

    return telegram;
}

struct mb_telegram read(struct mb_ctx ctx, uint8_t reg_type, uint16_t address, uint8_t count) {
    struct mb_telegram telegram = init_telegram();

    *telegram.s_id = ctx.s_id;
    *telegram.mb_fn_code = reg_type;
    *telegram.first_word = address;
    *telegram.second_word = count;

    *telegram.crc = calculate_crc(&telegram);

    return telegram;
}

struct mb_telegram read_do(struct mb_ctx ctx, uint16_t address, uint8_t count) {
    struct mb_telegram telegram = read(ctx, MB_FN_READ_DO, address, count);

    return telegram;
}

struct mb_telegram read_di(struct mb_ctx ctx, uint16_t address, uint8_t count) {

    struct mb_telegram telegram = read(ctx, MB_FN_READ_DI, address, count);

    return telegram;
}

struct mb_telegram read_ao(struct mb_ctx ctx, uint16_t address, uint8_t count) {
    struct mb_telegram telegram = read(ctx, MB_FN_READ_AO, address, count);

    return telegram;
}

struct mb_telegram read_ai(struct mb_ctx ctx, uint16_t address, uint8_t count) {
    struct mb_telegram telegram = read(ctx, MB_FN_READ_AI, address, count);

    return telegram;
}

struct mb_telegram write_single_do(struct mb_ctx ctx, uint16_t address, bool value) {
    struct mb_telegram telegram = init_telegram();
    *telegram.s_id = ctx.s_id;
    *telegram.mb_fn_code = MB_FN_WRITE_DO;
    *telegram.first_word = address;
    *telegram.second_word = value ? 0xFF : 0x00;
    *telegram.crc = calculate_crc(&telegram);

    return telegram;
}

struct mb_telegram write_single_ao(struct mb_ctx ctx, uint16_t address, const uint16_t value) {
    struct mb_telegram telegram = init_telegram();
    *telegram.s_id = ctx.s_id;
    *telegram.mb_fn_code = MB_FN_WRITE_AO;
    *telegram.first_word = address;
    *telegram.second_word = value;
    *telegram.crc = calculate_crc(&telegram);

    return telegram;
}

struct mb_telegram write_multiple_do(struct mb_ctx ctx, uint16_t address, const bool *values, uint8_t length) {

    uint8_t reg_count = length / 8;
    if ( (length % 8) != 0) reg_count += 1;

    struct mb_telegram telegram = init_telegram_data_ctrl_bytes(reg_count, 9);

    *telegram.s_id = ctx.s_id;
    *telegram.mb_fn_code = MB_FN_WRITE_MULTIPLE_DO;
    *telegram.first_word = address;
    *telegram.second_word = length;

    telegram.payload[6] = reg_count;

    uint8_t *ptr_start_ret = telegram.payload + 7;

    for (int i = 0; i < length; ++i) {
        uint8_t byte_index = (i / 8);
        uint8_t bit_index = (i % 8);
        if (bit_index == 0) { // Initialize Value with 0x00
            *(ptr_start_ret + byte_index) = 0x00;
        }
        // Set bit according to bool value
        *(ptr_start_ret + byte_index) = *(ptr_start_ret + byte_index) | (values[i] ? 0x01 : 0x00) << bit_index;
    }

    *telegram.crc = calculate_crc(&telegram);

    return telegram;
}

struct mb_telegram write_multiple_ao(struct mb_ctx ctx, uint16_t address, const uint16_t *values, uint8_t length) {

    struct mb_telegram telegram = init_telegram_data(length*2);
    *telegram.s_id = ctx.s_id;
    *telegram.mb_fn_code = MB_FN_WRITE_MULTIPLE_AO;
    *telegram.first_word = address;
    *telegram.second_word = length;
    for (int i = 1; i <= length; ++i) {
        // TODO Implement endianness
        *(telegram.second_word + (i * 2) ) = values[i];
    }

    *telegram.crc = calculate_crc(&telegram);

    return telegram;
}

struct mb_telegram reply_read_do(struct mb_ctx ctx, struct mb_telegram request) {
    if (*request.mb_fn_code != MB_FN_READ_DO) {
        fprintf(stderr, "Telegram not matching with called function (%d)!\n", MB_FN_READ_DO);
    }

    uint8_t *ptr_0 = request.payload;
    uint8_t *ptr_4 = ptr_0 + 4;
    uint8_t *ptr_5 = ptr_4 + 1;
    uint8_t coil_count = (0x0000 | (*ptr_4 << 8) | *ptr_5);

    uint8_t byte_count = coil_count / 8 + ((coil_count % 8 == 0) ? 0 : 1) ;



    struct mb_telegram telegram = init_telegram_data_ctrl_bytes(byte_count, 5);
    *telegram.s_id = ctx.s_id;
    *telegram.mb_fn_code = MB_FN_READ_DO;
    telegram.payload[2] = byte_count;

    uint8_t offset = 3;
    for (int i = 0; (i < coil_count) && ((coil_count + i) < ctx.count_do); ++i ) {
        uint8_t byte = ctx.reg_do[(*request.first_word) + i];
        telegram.payload[i+offset] = reverse_byte(byte);
    }
    uint8_t *last = telegram.payload + (2 + coil_count);

    /// Clear all non requestet bits
    /// bitwise and with 0xFF shifted by overhead of bits in last word
    *last = *last & (0xFF << (8 - (coil_count % 8) ) );

    uint16_t crc = calculate_crc(&telegram);

    *telegram.crc = crc;

    return telegram;
}

struct mb_telegram reply_read_di(struct mb_ctx ctx, struct mb_telegram request) {
    if (*request.mb_fn_code != MB_FN_READ_DI) {
        fprintf(stderr, "Telegram not matching with called function (%d)!\n", MB_FN_READ_DI);
    }

    uint8_t *ptr_0 = request.payload;
    uint8_t *ptr_4 = ptr_0 + 4;
    uint8_t *ptr_5 = ptr_4 + 1;
    uint8_t coil_count = (0x0000 | (*ptr_4 << 8) | *ptr_5);

    uint8_t byte_count = coil_count / 8 + ((coil_count % 8 == 0) ? 0 : 1) ;



    struct mb_telegram telegram = init_telegram_data_ctrl_bytes(byte_count, 5);
    *telegram.s_id = ctx.s_id;
    *telegram.mb_fn_code = MB_FN_READ_DI;
    telegram.payload[2] = byte_count;

    uint8_t offset = 3;
    for (int i = 0; (i < coil_count) && ((coil_count + i) < ctx.count_do); ++i ) {
        uint8_t byte = ctx.reg_di[(*request.first_word) + i];
        telegram.payload[i+offset] = reverse_byte(byte);
    }
    uint8_t *last = telegram.payload + (2 + coil_count);

    /// Clear all non requestet bits
    /// bitwise and with 0xFF shifted by overhead of bits in last word
    *last = *last & (0xFF << (8 - (coil_count % 8) ) );

    uint16_t crc = calculate_crc(&telegram);

    *telegram.crc = crc;

    return telegram;
}

struct mb_telegram reply_read_ao(struct mb_ctx ctx, struct mb_telegram request) {
    if (*request.mb_fn_code != MB_FN_READ_AO) {
        fprintf(stderr, "Telegram not matching with called function (%d)!\n", MB_FN_READ_AO);
    }

    uint16_t second_w = *request.second_word;
    uint16_t count_w = (uint8_t) switch_bytes(second_w);
    uint16_t count_b = count_w * 2;

    struct mb_telegram telegram = init_telegram_data_ctrl_bytes(count_b, 5);
    *telegram.s_id = ctx.s_id;
    *telegram.mb_fn_code = MB_FN_READ_AO;

    telegram.payload[2] = count_b;

    uint8_t offset = 3;
    for (int i = 0; (i < count_b) && ((count_b + i) < ctx.count_ao); ++i ) {
        telegram.payload[i+offset] = ctx.reg_ao[*request.first_word + i];
    }

    *telegram.crc = calculate_crc(&telegram);

    return telegram;
}

struct mb_telegram reply_read_ai(struct mb_ctx ctx, struct mb_telegram request) {
    if (*request.mb_fn_code != MB_FN_READ_AI) {
        fprintf(stderr, "Telegram not matching with called function (%d)!\n", MB_FN_READ_AI);
    }

    uint16_t second_w = *request.second_word;
    uint16_t count_w = (uint8_t) switch_bytes(second_w);
    uint16_t count_b = count_w * 2;

    struct mb_telegram telegram = init_telegram_data_ctrl_bytes(count_b, 5);
    *telegram.s_id = ctx.s_id;
    *telegram.mb_fn_code = MB_FN_READ_AI;

    telegram.payload[2] = count_b;

    uint8_t offset = 3;
    for (int i = 0; (i < count_b) && ((count_b + i) < ctx.count_ai); ++i ) {
        telegram.payload[i+offset] = ctx.reg_ai[*request.first_word + i];
    }

    *telegram.crc = calculate_crc(&telegram);

    return telegram;
}

struct mb_telegram reply_write_single_do(struct mb_ctx ctx, struct mb_telegram request) {
    // TODO MB Exception
    if (*request.mb_fn_code != MB_FN_WRITE_DO) {
        fprintf(stderr, "Telegram not matching with called function (%d)!\n", MB_FN_WRITE_DO);
    }

    uint16_t address_reg = *request.first_word;
    uint8_t byte_index = switch_bytes(address_reg) / 8;
    if (ctx.count_do > byte_index) {

        uint8_t bit_index = 7 - (address_reg % 8);

        bool value = *request.second_word != 0;
        uint8_t current_value = ctx.reg_do[byte_index];

        // https://stackoverflow.com/a/47990
        uint8_t byte = (current_value & ~((uint8_t)1 << bit_index)) | ((uint8_t)value << bit_index);

        ctx.reg_do[byte_index] = byte;
    }

    return request;
}

struct mb_telegram reply_write_single_ao(struct mb_ctx ctx, struct mb_telegram request) {
    // TODO MB Exception
    if (*request.mb_fn_code != MB_FN_WRITE_AO) {
        fprintf(stderr, "Telegram not matching with called function (%d)!\n", MB_FN_WRITE_AO);
    }

    uint16_t address_reg = *request.first_word;
    if (ctx.count_ao > address_reg) {
        ctx.reg_ao[address_reg] = switch_bytes(*request.second_word);
    }

    return request;
}

struct mb_telegram reply_write_multiple_do(struct mb_ctx ctx, struct mb_telegram request) {
    // TODO MB Exception
    if (*request.mb_fn_code != MB_FN_WRITE_MULTIPLE_DO) {
        fprintf(stderr, "Telegram not matching with called function (%d)!\n", MB_FN_WRITE_MULTIPLE_DO);
    }

    uint16_t address_reg = *request.first_word;
    uint8_t reg_count = *request.second_word;


    uint8_t *ptr_data = request.payload + 7;
    for (int i = 0; i < reg_count; ++i) {
        if (ctx.count_do > (address_reg + i) ) {
            break;
        }
        ctx.reg_do[address_reg + i] = ptr_data[i];
    }

    struct mb_telegram response = init_telegram();

    *response.s_id = ctx.s_id;
    *response.mb_fn_code = MB_FN_WRITE_MULTIPLE_DO;
    *response.first_word = address_reg;
    *response.second_word = reg_count;

    *response.crc = calculate_crc(&response);


    return response;
}

struct mb_telegram reply_write_multiple_ao(struct mb_ctx ctx, struct mb_telegram request) {
    // TODO MB Exception
    if (*request.mb_fn_code != MB_FN_WRITE_MULTIPLE_AO) {
        fprintf(stderr, "Telegram not matching with called function (%d)!\n", MB_FN_WRITE_MULTIPLE_AO);
    }

    uint16_t address_reg = switch_bytes(*request.first_word);
    uint8_t reg_count = switch_bytes(*request.second_word);


    uint16_t *ptr_data = (uint16_t*) (request.payload + 7);
    for (int i = 0; i < reg_count; ++i) {
        if (ctx.count_ao > (address_reg + i) ) {
            break;
        }
        ctx.reg_do[address_reg + i] = ptr_data[i];
    }

    struct mb_telegram response = init_telegram();

    *response.s_id = ctx.s_id;
    *response.mb_fn_code = MB_FN_WRITE_MULTIPLE_AO;
    *response.first_word = *request.first_word;
    *response.second_word = *request.second_word;

    *response.crc = calculate_crc(&response);


    return response;
}

struct mb_telegram mb_exception(uint8_t mb_fn, uint8_t mb_exception) {
    if (false) {

    }
    struct mb_telegram t;
    return t;
}

struct mb_telegram reply(struct mb_ctx ctx, struct mb_telegram request) {
    struct mb_telegram result;
    switch ((enum MB_FN) *request.mb_fn_code) {

        case READ_DO:
            result = reply_read_do(ctx, request);
            break;
        case READ_DI:
            result = reply_read_di(ctx, request);
            break;
        case READ_AO:
            result = reply_read_ao(ctx, request);
            break;
        case READ_AI:
            result = reply_read_ai(ctx, request);
            break;
        case WRITE_DO:
            result = reply_write_single_do(ctx, request);
            break;
        case WRITE_AO:
            result = reply_write_single_ao(ctx, request);
            break;
        case WRITE_MULTIPLE_DO:
            result = reply_write_multiple_do(ctx, request);
            break;
        case WRITE_MULTIPLE_AO:
            result = reply_write_multiple_ao(ctx, request);
            break;
    }

    return result;
}
