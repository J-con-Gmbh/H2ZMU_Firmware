//
// Created by jriessner on 09.02.24.
//

#ifndef TEST_MB_DEFINITIONS_H
#define TEST_MB_DEFINITIONS_H

#define MB_FN_01 0x01
#define MB_FN_READ_DO MB_FN_01

#define MB_FN_02 0x02
#define MB_FN_READ_DI MB_FN_02

#define MB_FN_03 0x03
#define MB_FN_READ_AO MB_FN_03

#define MB_FN_04 0x04
#define MB_FN_READ_AI MB_FN_04

#define MB_FN_05 0x05
#define MB_FN_WRITE_DO MB_FN_05

#define MB_FN_06 0x06
#define MB_FN_WRITE_AO MB_FN_06

#define MB_FN_15 0x0F
#define MB_FN_WRITE_MULTIPLE_DO MB_FN_15

#define MB_FN_16 0x10
#define MB_FN_WRITE_MULTIPLE_AO MB_FN_16



#define MB_EXC_01 0x01
#define MB_EXC_ILLEGAL_FUNCTION MB_EXC_01

#define MB_EXC_02 0x02
#define MB_EXC_ILLEGAL_DATA_ACCESS MB_EXC_02

#define MB_EXC_03 0x03
#define MB_EXC_ILLEGAL_DATA_VALUE MB_EXC_03

#define MB_EXC_04 0x04
#define MB_EXC_SLAVE_DEVICE_FAILURE MB_EXC_04

#define MB_EXC_05 0x05
#define MB_EXC_ACKNOWLEDGE MB_EXC_05

#define MB_EXC_06 0x06
#define MB_EXC_SLAVE_DEVICE_BUSY MB_EXC_06

#define MB_EXC_07 0x07
#define MB_EXC_NEGATIVE_ACKNOWLEDGEMENT MB_EXC_07

#define MB_EXC_08 0x08
#define MB_EXC_MEMORY_PARITY_ERROR MB_EXC_08

#define MB_EXC_10 0x0A
#define MB_EXC_GATEWAY_PATH_UNAVAILABLE MB_EXC_10

#define MB_EXC_11 0x0B
#define MB_EXC_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND MB_EXC_11



#endif //TEST_MB_DEFINITIONS_H