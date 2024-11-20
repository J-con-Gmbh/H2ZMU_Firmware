//
// Created by jriessner on 09.02.24.
//

#ifndef MODBUS_PNP_MODBUS_PNP_H
#define MODBUS_PNP_MODBUS_PNP_H


#include <stdbool.h>
#include "mb_structs.h"
#include "mb_definitions.h"

/**
 * https://stackoverflow.com/a/2602885
 * @param b
 * @return
 */
uint8_t reverse_byte(uint8_t b);

uint16_t switch_bytes(uint16_t w);

/**
 * https://ctlsys.com/support/how_to_compute_the_modbus_rtu_message_crc/
 *
 * @param buf
 * @param len
 * @return crc
 */
uint16_t calculate_crc(const struct mb_telegram* telegram);

struct mb_ctx init_context(uint8_t s_id, uint8_t count_do, uint8_t count_di, uint8_t count_ao, uint8_t count_ai);
void free_context(struct mb_ctx ctx);

struct mb_telegram init_telegram_data_ctrl_bytes(uint8_t data_len, uint8_t count_ctl_bytes);
struct mb_telegram init_telegram_data(uint8_t data_len);
struct mb_telegram init_telegram();

struct mb_telegram mb_exception(uint8_t mb_fn, uint8_t mb_exception);

struct mb_telegram parse_telegram(const uint8_t* data, uint8_t data_len);

struct mb_telegram read(struct mb_ctx ctx, uint8_t reg_type, uint16_t address, uint8_t count);

/**
 * Read discrete output (read-write) coils. <br>
 * @param address
 * @param count
 * @return bool success
 */
struct mb_telegram read_do(struct mb_ctx ctx, uint16_t address, uint8_t count);

/**
 * Read discrete input (readonly) contacts. <br>
 * @param address
 * @param count
 * @return bool success
 */
struct mb_telegram read_di(struct mb_ctx ctx, uint16_t address, uint8_t count);

/**
 * Read analog input (readonly) registers. <br>
 * @param address
 * @param count
 * @return bool success
 */
struct mb_telegram read_ao(struct mb_ctx ctx, uint16_t address, uint8_t count);

/**
 * Read analog output (read-write) registers. <br>
 *
 * @param address
 * @param count
 * @return bool success
 */
struct mb_telegram read_ai(struct mb_ctx ctx, uint16_t address, uint8_t count);

struct mb_telegram write_single_do(struct mb_ctx ctx, uint16_t address, bool value);

struct mb_telegram write_single_ao(struct mb_ctx ctx, uint16_t address, uint16_t value);

struct mb_telegram write_multiple_do(struct mb_ctx ctx, uint16_t address, const bool *values, uint8_t length);

struct mb_telegram write_multiple_ao(struct mb_ctx ctx, uint16_t address, const uint16_t* values, uint8_t length);

struct mb_telegram reply(struct mb_ctx ctx, struct mb_telegram request);

struct mb_telegram reply_read_do(struct mb_ctx ctx, struct mb_telegram request);
struct mb_telegram reply_read_di(struct mb_ctx ctx, struct mb_telegram request);
struct mb_telegram reply_read_ao(struct mb_ctx ctx, struct mb_telegram request);
struct mb_telegram reply_read_ai(struct mb_ctx ctx, struct mb_telegram request);

struct mb_telegram reply_write_single_do(struct mb_ctx ctx, struct mb_telegram request);
struct mb_telegram reply_write_single_ao(struct mb_ctx ctx, struct mb_telegram request);

struct mb_telegram reply_write_multiple_do(struct mb_ctx ctx, struct mb_telegram request);
struct mb_telegram reply_write_multiple_ao(struct mb_ctx ctx, struct mb_telegram request);



#endif //MODBUS_PNP_MODBUS_PNP_H
