//
// Created by jriessner on 01.09.23.
//

#include "interface/modbus/PlcInterface.h"

#include "Core.h"

#include "data/db/repositories/ParamRepository.h"
#include "data/db/repositories/CascadeRepository.h"
#include "data/db/repositories/BottleRepository.h"
#include "data/db/repositories/SensorRepository.h"
#include "data/db/repositories/OccurredErrorRepository.h"
#include "data/db/repositories/OccurredWarningRepository.h"
#include "data/param/ParameterInterface.h"
#include "data/db/repositories/TranslationRepository.h"
#include "epoch_time.h"
#include "utilities/modbus/ModbusDataConversion.h"
#include "data/db/entities/e_Cascade.h"
#include "process_logic/initialsetup/InitialSetupDataCache.h"
#include <iostream>
#include <string>

std::vector<uint16_t> PlcInterface::mb_startMeasurement(mb::modbus_register_data *data, const mb::telegram &t) {

    if (!Core::instance->control.claimControl(PlcInterface::control)) {
        // TODO Throw Modbus Error
        return { (uint16_t) false};
    }

    if (
            !t.data.empty()
            && (t.function_code == 0x05 && !PlcInterface::measurement_start)
            && t.data[0]
            && !Core::instance->getBlockMeas()
        ) {

        PlcInterface::measurement_start = true;

        Core::instance->control.send(PlcInterface::control, core::control::action::MEASUREMENT_START,
                                     PlcInterface::measurement_externalId);

        PlcInterface::measurement_externalId = "";
    }

    return { (uint16_t) measurement_start};
}

std::vector<uint16_t> PlcInterface::mb_stopMeasurement(mb::modbus_register_data *data, const mb::telegram &t) {

    if (
            !t.data.empty()
            && (t.function_code == 0x05 && !PlcInterface::measurement_stop)
            && t.data[0] != 0
        ) {

        PlcInterface::measurement_stop = true;

        Core::instance->control.send(PlcInterface::control, core::control::action::MEASUREMENT_STOP);
    }

    return { (uint16_t) measurement_stop};
}

std::vector<uint16_t> PlcInterface::mb_getDaytime(mb::modbus_register_data *data, const mb::telegram &t) {
    std::string date = utils::epoch_time::getTimestamp("%Y-%m-%d");
    std::string daytime = utils::epoch_time::getTimestamp("%T");

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        std::string new_daytime = registerDataToString(regData);
        std::string full_datetime_string = date + " " + new_daytime;
        std::string syscall_string = "date -s " + '"' + full_datetime_string + '"';
        system(syscall_string.c_str());
        system("hwclock -w");
        
    }

    return ModbusServer::stringToRegisterData(daytime);
}

std::vector<uint16_t> PlcInterface::mb_getDate(mb::modbus_register_data *data, const mb::telegram &t) {
    std::string date = utils::epoch_time::getTimestamp("%Y-%m-%d");
    std::string daytime = utils::epoch_time::getTimestamp("%T");

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        std::string new_date = registerDataToString(regData);
        std::string full_datetime_string = new_date + " " + daytime;
        std::string syscall_string = "date -s " + '"' + full_datetime_string + '"';
        system(syscall_string.c_str());
        system("hwclock -w");
    }

    return ModbusServer::stringToRegisterData(date);
}

std::vector<uint16_t> PlcInterface::mb_readWriteParameter(mb::modbus_register_data *data, const mb::telegram &t) {

    param parameter;
    try {
        parameter = ParamRepository::instance->getParamById(std::stoi(data->function_pointer.ptr_arg));
    } catch (std::exception &e) {
        std::cerr << "mb_readWriteParameter function pointer ptr_arg not parsable!" << std::endl;
        return {};
    }

    if (t.function_code > 0x04) {
        union {
            uint16_t value;
            uint8_t split[2];
        };
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i) {
            unsigned char item = t.data[i];
            /*
             * regData is initialized with 0x0000 in every place.
             * Every first iteration, the first byte of the uint16_t is written,
             * every second iteration the second byte is written.
             * After that, the next uint16_t is set.
             */
            // regData[i/2] set to current value binary or'ed with data-byte-value, offset by 8 if is second iteration on same uint16_t
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }
        auto p = registerDataToParameter(regData, parameter);
        if (!ParamRepository::instance->updateParam(p)) {
            //TODO Fehlermeldung Parameter update fehlgeschlagen!
            std::cerr << "Parameter update fehlgeschlagen!" << std::endl;
        }
    }
    std::vector<uint16_t> ret {};
    try {
        ret = parameterToRegisterData(ParamRepository::instance->getParamById(std::stoi(data->function_pointer.ptr_arg)));
    } catch (std::exception &e) {
        std::cerr << "mb_readWriteParameter function pointer ptr_arg not parsable!" << std::endl;
    }

    return ret;
}

void PlcInterface::defineDataStructs() {

    auto functionPtrToPlcData = [](uint16_t &reg_addr,
                                   int nr,
                                   const std::string &short_desc,
                                   const std::string &long_desc,
                                   const mb::modbus_function_pointer &function_pointer,
                                   enum usergroup access_read_restriction = USER,
                                   enum usergroup access_write_restriction = USER,
                                   enum mb::modbus_register_type register_type = mb::AO,
                                   enum mb::modbus_address_direction data_direction = mb::OUTPUT,
                                   bool leave_blank = false
    ) {

        uint8_t reg_len = (function_pointer.type_size / 2) + (function_pointer.type_size % 2);

        struct plc_data data = {
                .nr = nr,
                .short_desc = short_desc,
                .long_desc = long_desc,
                .access_read = access_read_restriction,
                .access_write = access_write_restriction,
                .register_data = {
                        .type = mb::FUNCTION_POINTER,
                        .register_type = register_type,
                        .direction = data_direction,
                        .address = reg_addr,
                        .total_register_length = reg_len,
                        .function_pointer = function_pointer
                }
        };
        if (leave_blank) {
            data.register_data.function_pointer.ptr = PlcInterface::mb_returnBlank;
        }

        reg_addr += reg_len;

        return data;
    };


    // TODO Saubere Implementierung des Registermappings
    uint16_t reg = 40001;
    int dataMapIndex = 0;

    this->dataMap[dataMapIndex++] = plc_data {.nr = -100, .short_desc = "MS-S", .long_desc = "Messpunkt Start", .access_read = USER, .access_write = USER, .register_data {
        .type = mb::FUNCTION_POINTER,
        .register_type = mb::DO,
        .direction = mb::BIDIRECTIONAL,
        .address = 1,
        .function_pointer {
            .type = mb::BOOL,
            .type_size = 1,
            .ptr = PlcInterface::mb_startMeasurement
        }
    }};

    this->dataMap[dataMapIndex++] = plc_data {.nr = -99, .short_desc = "MS-E", .long_desc = "Messpunkt Stop", .access_read = USER, .access_write = USER, .register_data {
        .type = mb::FUNCTION_POINTER,
        .register_type = mb::DO,
        .direction = mb::BIDIRECTIONAL,
        .address = 2,
        .function_pointer {
            .type = mb::BOOL,
            .type_size = 1,
            .ptr = PlcInterface::mb_stopMeasurement
        }
    }};

    this->dataMap[dataMapIndex++] = plc_data {.nr = -98, .short_desc = "EXT-ID", .long_desc = "Externe Messungsnummer", .access_read = USER, .access_write = USER, .register_data {
        .type = mb::POINTER,
        .register_type = mb::AO,
        .direction = mb::BIDIRECTIONAL,
        .address = reg,
        .total_register_length = 10,
        .max_string_length = 20,
        .data_pointer {
            .type = mb::STRING,
            .type_size = 20,
            .ptr = (void*) &PlcInterface::measurement_externalId
        }
    }};


    // TODO User rights !!!!!
    this->dataMap[dataMapIndex++] = plc_data{.nr = -50, .short_desc = "FW-UP", .long_desc = "Firmware Update", .access_read = USER, .access_write = USER, .register_data {
            .type = mb::FUNCTION_POINTER,
            .register_type = mb::DO,
            .direction = mb::BIDIRECTIONAL,
            .address = 10,
            .function_pointer {
                    .type = mb::BOOL,
                    .type_size = 1,
                    .ptr = PlcInterface::mb_receiveFirmwareUpdate
            }
    }};

    reg += 1;

    reg = 40100;
    // TODO Function Pointer not working properly
    // TODO Multi-language

    std::map<int, struct cascade> allCascades = CascadeRepository::instance->getAllCascades();
    std::map<int, struct sensor> allTempsensors = SensorRepository::instance->getAllTempSensors();
    std::map<int, struct sensor> allPsensors = SensorRepository::instance->getAllPressureSensors();
    std::map<int, struct bottle> allBottles = BottleRepository::instance->getAllBottles();
    std::map<int, struct occurrederror> allErrors = OccurredErrorRepository::instance->getAllOccurredErrors();
    std::map<int, struct occurredwarning> allWarnings = OccurredWarningRepository::instance->getAllOccurredWarnings();

    // Stammdaten Gerät
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1, "D-N", "Gerätename",                 {.type = mb::STRING,  .type_size = 20, .ptr = PlcInterface::mb_readWriteParameter,  .ptr_arg = std::to_string(1)}, USER ,SERVICE,  mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 2, "D-V", "Gerätevariante",             {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(2)}, USER, SERVICE,  mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 3, "D-SNR", "Seriennummer Gerät",       {.type = mb::STRING,  .type_size = 20,  .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(3)}, USER, SERVICE,  mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 4, "D-BJ", "Gerätebaujahr",             {.type = mb::INT16,  .type_size = 2,  .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(4)}, USER, SERVICE,  mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 5, "D-FW", "Firmwareversion",           {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(5)}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 6, "S-N", "Anlagenname",                {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(6)}, USER, SERVICE,  mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 7, "S-NR", "Nummer Anlage",             {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(7)}, USER, SERVICE,  mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 8, "D-CC", "Anzahl Kaskaden (max. 25)", {.type = mb::INT16, .type_size = 2, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(8)}, USER, SERVICE,  mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 9, "D-BA", "Betriebsart, 01 = Differenzmessung, 02= Einzelwertmessung", {.type = mb::INT16, .type_size = 2, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(9)}, USER, SERVICE,  mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 10, "D-LAN", "Gerätesprache", {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(10)}, USER, SERVICE,  mb::AO, mb::BIDIRECTIONAL);

    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 20, "MBS-B", "Modbus Server Baudrate", {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(20)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 21, "MBS-P", "Modbus Server Parität",  {.type = mb::CHAR,  .type_size = 2, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(21)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 22, "MBS-DB", "Modbus Server Datenbits", {.type = mb::INT16, .type_size = 2, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(22)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 23, "MBS-SB", "Modbus Server Stoppbit", {.type = mb::INT16, .type_size = 2, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(23)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 24, "MBS-SID", "Modbus Server-Id", {.type = mb::INT16, .type_size = 2, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(24)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 25, "MBCG-B", "Modbus Client Gateway Baudrate", {.type = mb::INT32,  .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(25)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 26, "MBCG-P", "Modbus Client Gateway Parität", {.type = mb::CHAR,  .type_size = 2, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(26)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 27, "MBCG-DB", "Modbus Client Gateway Datenbits", {.type = mb::INT16,  .type_size = 2, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(27)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 28, "MBCG-SB", "Modbus Client Gateway Stoppbit", {.type = mb::INT16,  .type_size = 2, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(28)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 29, "MBCG-SID", "Modbus Client Gateway-Id", {.type = mb::INT16,  .type_size = 2, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(29)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 30, "MBCS-B", "Modbus Client Waage Baudrate", {.type = mb::INT32,  .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(30)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 31, "MBCS-P", "Modbus Client Waage Parität", {.type = mb::CHAR,  .type_size = 2, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(31)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 32, "MBCS-DB", "Modbus Client Waage Datenbits", {.type = mb::INT16,  .type_size = 2, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(32)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 33, "MBCS-SB", "Modbus Client Waage Stoppbit", {.type = mb::INT16,  .type_size = 2, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(33)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 34, "MBCS-SID", "Modbus Client Waage-Id", {.type = mb::INT16,  .type_size = 2, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(34)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);

    // Stammdaten Medium
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 100, "b1", "Konstante Virialgleichung Normbedingungen b1", {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(100)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 101, "b2", "Konstante Virialgleichung Normbedingungen b2", {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(101)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 102, "b3", "Konstante Virialgleichung Normbedingungen b3", {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(102)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 103, "b4", "Konstante Virialgleichung Normbedingungen b4", {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(103)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 104, "b5", "Konstante Virialgleichung Normbedingungen b5", {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(104)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 105, "c1", "Konstante Virialgleichung Normbedingungen c1", {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(105)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 106, "c2", "Konstante Virialgleichung Normbedingungen c2", {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(106)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 107, "c3", "Konstante Virialgleichung Normbedingungen c3", {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(107)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 108, "c4", "Konstante Virialgleichung Normbedingungen c4", {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(108)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 109, "c5", "Konstante Virialgleichung Normbedingungen c5", {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(109)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 110, "DH2", "Dichte Medium", {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(110)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 120, "NCT", "Normtemperatur", {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(120)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 121, "NCp", "Normdruck", {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(121)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);

    // Statusinformation Bedienung
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 300, "SSW1", "Softwareschalter Bedienung, OFF = Gerät, ON = DFÜ", {.type = mb::BOOL, .type_size = 1, .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(300)}, USER, USER, mb::AO, mb::BIDIRECTIONAL);

    // Statusinformation DI/DO
    int register_it = 0;
    int iterator = 0;

    for(int i = 1; i < 5; ++i) {
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 400 + register_it, "DI-N" + std::to_string(i), "Name Signal DigitaleingangKanal " + std::to_string(i), {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getDate, .ptr_arg = std::to_string(i)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 401 + register_it, "DI-S" + std::to_string(i), "Signal DigitaleingangKanal " + std::to_string(i), {.type = mb::BOOL, .type_size = 1, .ptr = PlcInterface::mb_getDate, .ptr_arg = std::to_string(i)}, USER, READONLY, mb::AO, mb::OUTPUT);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 402 + register_it, "DO-N" + std::to_string(i), "Name Signal DigitalausgangKanal " + std::to_string(i), {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getDate, .ptr_arg = std::to_string(i)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 403 + register_it, "DO-S" + std::to_string(i), "Signal DigitalausgangKanal " + std::to_string(i), {.type = mb::BOOL, .type_size = 1, .ptr = PlcInterface::mb_getDate, .ptr_arg = std::to_string(i)}, USER, READONLY, mb::AO, mb::OUTPUT);
        register_it = register_it + 10;
    }

    // Statusinformation Einzelwertmessung
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 500, "EM-TIME-START", "Zeitstempel Start",         {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getLastMeasurementStart}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 501, "EM-TIME-STOP", "Zeitstempel Stop",         {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getLastMeasurementStart}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 502, "EM-DATE-START", "Datumsstempel Start",    {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getLastMeasurementStart}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 503, "EM-DATE-STOP", "Datumsstempel Stop",    {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getLastMeasurementStart}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 504, "EM-MNR", "Aktuelle Messungsnummer", {.type = mb::INT32,  .type_size = 4,  .ptr = PlcInterface::mb_getCurrentMeasurementId}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 505, "CC", "Aktuelle Kaskadennummer, 0 = Differenzm., n = Einzelwertm.", {.type = mb::INT32,  .type_size = 4,  .ptr = PlcInterface::mb_getCascadeCount}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 510, "TM-m", "Aktuelle Masse Kaskade", {.type = mb::FLOAT32,  .type_size = 4,  .ptr = PlcInterface::mb_getCascadeMassTotal}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 511, "TM-V", "Aktuelles Volumen Kaskade", {.type = mb::FLOAT32,  .type_size = 4,  .ptr = PlcInterface::mb_getCascadeCount}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 512, "TM-T", "Aktuelle Temperatur Kaskade", {.type = mb::FLOAT32,  .type_size = 4,  .ptr = PlcInterface::mb_getCascadeCount}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 513, "TM-P", "Aktueller Druck Kaskade", {.type = mb::FLOAT32,  .type_size = 4,  .ptr = PlcInterface::mb_getCascadeCount}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 514, "TM-EC", "Aktuelle Fehlermeldung ", {.type = mb::STRING,  .type_size = 20,  .ptr = PlcInterface::mb_getCascadeCount}, USER, READONLY, mb::AO, mb::OUTPUT);

    // Statusinformation Differenzmessung
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 600, "DM-MNR", "Aktuelle Messungsnummer", {.type = mb::INT32,  .type_size = 4,  .ptr = PlcInterface::mb_getCurrentMeasurementId}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 601, "DM-TIME-START", "Zeitstempel Start",         {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getLastMeasurementStart}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 602, "DM-TIME-STOP", "Zeitstempel Stop",         {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getLastMeasurementEnd}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 603, "DM-DATE-START", "Datumsstempel Start",    {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getLastMeasurementStart}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 604, "DM-DATE-STOP", "Datumsstempel Stop",    {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getLastMeasurementEnd}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 605, "TM-SEC", "Aktuelle Fehlermeldung Start", {.type = mb::STRING,  .type_size = 20,  .ptr = PlcInterface::mb_getCascadeCount}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 606, "TM-EEC", "Aktuelle Fehlermeldung Ende", {.type = mb::STRING,  .type_size = 20,  .ptr = PlcInterface::mb_getCascadeCount}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 610, "TM-dmS", "Aktuelle Massendifferenz über alle Kaskaden", {.type = mb::FLOAT32,  .type_size = 4,  .ptr = PlcInterface::mb_getLastMeasurementMass}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 611, "TM-SmS", "Aktuelle Masse Start über alle Kaskaden", {.type = mb::FLOAT32,  .type_size = 4,  .ptr = PlcInterface::mb_getLastMeasurementMassStart}, USER, READONLY, mb::AO, mb::OUTPUT);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 612, "TM-EmS", "Aktuelle Masse Ende über alle Kaskaden", {.type = mb::FLOAT32,  .type_size = 4,  .ptr = PlcInterface::mb_getLastMeasurementMassEnd}, USER, READONLY, mb::AO, mb::OUTPUT);

    register_it = 0;
    iterator = 0;

    for (const auto &_cascade: allCascades)
    {
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 620 + iterator, "TM-dmK" + std::to_string(iterator + 1), "Aktuelle Massendifferenz Kaskade " + std::to_string(iterator + 1), {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_getCascadeMassDifference, .ptr_arg = std::to_string(_cascade.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 650 + iterator, "TM-SmK" + std::to_string(iterator + 1), "Aktuelle Masse Start Kaskade " + std::to_string(iterator + 1), {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_getCascadeStartMass, .ptr_arg = std::to_string(_cascade.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 680 + iterator, "TM-EmK" + std::to_string(iterator + 1), "Aktuelle Masse Ende Kaskade " + std::to_string(iterator + 1), {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_getCascadeEndMass, .ptr_arg = std::to_string(_cascade.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 710 + iterator, "TM-SV" + std::to_string(iterator + 1), "Aktuelles Volumen Start Kaskade " + std::to_string(iterator + 1), {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_getCascadeStartVolumeCorrected, .ptr_arg = std::to_string(_cascade.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 740 + iterator, "TM-EV" + std::to_string(iterator + 1), "Aktuelles Volumen Ende Kaskade " + std::to_string(iterator + 1), {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_getCascadeEndVolumeCorrected, .ptr_arg = std::to_string(_cascade.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 770 + iterator, "TM-ST" + std::to_string(iterator + 1), "Aktuelle Temperatur Start Kaskade " + std::to_string(iterator + 1), {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_getCascadeStartTemperature, .ptr_arg = std::to_string(_cascade.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 800 + iterator, "TM-ET" + std::to_string(iterator + 1), "Aktuelle Temperatur Ende Kaskade " + std::to_string(iterator + 1), {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_getCascadeEndTemperature, .ptr_arg = std::to_string(_cascade.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 830 + iterator, "TM-Sp" + std::to_string(iterator + 1), "Aktueller Druck Start Kaskade " + std::to_string(iterator + 1), {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_getCascadeStartPressure, .ptr_arg = std::to_string(_cascade.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 860 + iterator, "TM-Ep" + std::to_string(iterator + 1), "Aktueller Druck Ende Kaskade " + std::to_string(iterator + 1), {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_getCascadeEndPressure, .ptr_arg = std::to_string(_cascade.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 890 + iterator, "TM-Sz" + std::to_string(iterator + 1), "Aktuelle Kompressibilität Start Kaskade " + std::to_string(iterator + 1), {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_getCascadeStartRGFS, .ptr_arg = std::to_string(_cascade.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 920 + iterator, "TM-Ez" + std::to_string(iterator + 1), "Aktuelle Kompressibilität Ende Kaskade " + std::to_string(iterator + 1), {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_getCascadeEndRGFS, .ptr_arg = std::to_string(_cascade.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);
        iterator++; 
    }

    iterator = 0;

    //Stammdaten Druckmessumformer
    for (const auto &_psensor: allPsensors)   
    {
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1000 + iterator, "PT-N" + std::to_string(iterator + 1), "Nummer Druckmessumformer (000 + Kaskade 01-25) " +  std::to_string(iterator + 1), {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_getTempSensorCascade, .ptr_arg = std::to_string(_psensor.second.id)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1030 + iterator, "PT-SNR" + std::to_string(iterator + 1), "Seriennummer Druckmessumformer " + std::to_string(iterator + 1), {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getTempSensorSerial, .ptr_arg = std::to_string(_psensor.second.id)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1060 + iterator, "PT-MBL" + std::to_string(iterator + 1), "Untere Messbereichsgrenze Druckmessumformer (Hersteller) " + std::to_string(iterator + 1), {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_getTempSensorLowerLimitManufacturer, .ptr_arg = std::to_string(_psensor.second.id)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1090 + iterator, "PT-MBU" + std::to_string(iterator + 1), "Obere Messbereichsgrenze Druckmessumformer (Hersteller) " + std::to_string(iterator + 1), {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_getTempSensorUpperLimitManufacturer, .ptr_arg = std::to_string(_psensor.second.id)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1120 + iterator, "PT-MBLU" + std::to_string(iterator + 1), "Untere Messbereichsgrenze Druckmessumformer (Benutzer) " + std::to_string(iterator + 1), {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_getTempSensorLowerLimitUser, .ptr_arg = std::to_string(_psensor.second.id)}, USER, USER, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1150 + iterator, "P-MBUUT" + std::to_string(iterator + 1), "Obere Messbereichsgrenze Druckmessumformer (Benutzer) " + std::to_string(iterator + 1), {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_getTempSensorUpperLimitUser, .ptr_arg = std::to_string(_psensor.second.id)}, USER, USER, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1180 + iterator, "PT-DCAL" + std::to_string(iterator + 1), "Kalibrierdatum Drucktransmitter " + std::to_string(iterator + 1), {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getTempSensorCalibrationDate, .ptr_arg = std::to_string(_psensor.second.id)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1210 + iterator, "PT-CORM" + std::to_string(iterator + 1), "Kalibrier-Korrekturgerade M " + std::to_string(iterator + 1), {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_getTempSensorCorrectionM, .ptr_arg = std::to_string(_psensor.second.id)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1240 + iterator, "PT-CORB" + std::to_string(iterator + 1), "Kalibrier-Korrekturgerade B " + std::to_string(iterator + 1), {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_getTempSensorCorrectionB, .ptr_arg = std::to_string(_psensor.second.id)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
    }

    iterator = 0;

    //Stammdaten Temperaturmessumformer
    for (const auto &_tempsensor: allTempsensors)   
    {
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1500 + iterator, "TT-N" + std::to_string(iterator + 1), "Nummer Temperaturmessumformer (000 + Kaskade 01-25) " + std::to_string(iterator + 1), {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_getTempSensorCascade, .ptr_arg = std::to_string(_tempsensor.second.id)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1530 + iterator, "TT-SNR" + std::to_string(iterator + 1), "Seriennummer Temperaturmessumformer " + std::to_string(iterator + 1), {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getTempSensorSerial, .ptr_arg = std::to_string(_tempsensor.second.id)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1560 + iterator, "TT-MBL" + std::to_string(iterator + 1), "Untere Messbereichsgrenze Temperaturmessumformer (Hersteller) " + std::to_string(iterator + 1), {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_getTempSensorLowerLimitManufacturer, .ptr_arg = std::to_string(_tempsensor.second.id)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1590 + iterator, "TT-MBU" + std::to_string(iterator + 1), "Obere Messbereichsgrenze Temperaturmessumformer (Hersteller) " + std::to_string(iterator + 1), {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_getTempSensorUpperLimitManufacturer, .ptr_arg = std::to_string(_tempsensor.second.id)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1620 + iterator, "TT-MBLU" + std::to_string(iterator + 1), "Untere Messbereichsgrenze Temperaturmessumformer (Benutzer) " + std::to_string(iterator + 1), {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_getTempSensorLowerLimitUser, .ptr_arg = std::to_string(_tempsensor.second.id)}, USER, USER, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1650 + iterator, "TT-MBUUT" + std::to_string(iterator + 1), "Obere Messbereichsgrenze Temperaturmessumformer (Benutzer) " + std::to_string(iterator + 1), {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_getTempSensorUpperLimitUser, .ptr_arg = std::to_string(_tempsensor.second.id)}, USER, USER, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1680 + iterator, "TT-DCAL" + std::to_string(iterator + 1), "Kalibrierdatum Temperaturtransmitter " + std::to_string(iterator + 1), {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getTempSensorCalibrationDate, .ptr_arg = std::to_string(_tempsensor.second.id)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1710 + iterator, "TT-CORM" + std::to_string(iterator + 1), "Kalibrier-Korrekturgerade M " + std::to_string(iterator + 1), {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_getTempSensorCorrectionM, .ptr_arg = std::to_string(_tempsensor.second.id)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 1740 + iterator, "TT-CORB" + std::to_string(iterator + 1), "Kalibrier-Korrekturgerade B " + std::to_string(iterator + 1), {.type = mb::FLOAT32, .type_size = 4, .ptr = PlcInterface::mb_getTempSensorCorrectionB, .ptr_arg = std::to_string(_tempsensor.second.id)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);      
    }

    iterator = 0;

    // Stammdaten Kaskade
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 2000, "B-LM", "Grenzwert unterer zulässiger Druckmessbereich Kaskaden", {.type = mb::FLOAT32,  .type_size = 4,  .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(2000)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 2001, "B-PCT", "Grenzwert Druckänderung über der Zeit", {.type = mb::FLOAT32,  .type_size = 4,  .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(2001)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
    this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 2002, "B-TU", "Oberer Grenzwert Temperatur", {.type = mb::FLOAT32,  .type_size = 4,  .ptr = PlcInterface::mb_readWriteParameter, .ptr_arg = std::to_string(2002)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);

    for (const auto &_cascade: allCascades)
    {
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 2100 + iterator, "K-N" + std::to_string(iterator + 1), "Kaskade " + std::to_string(iterator + 1) + " Anzahl Gasflaschen", {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_getCascadeCountBottles, .ptr_arg = std::to_string(_cascade.second.id)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
        for (const auto &_bottle: allBottles) {
            if(_bottle.second.fk_cascade == _cascade.second.id) {
                this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 2201 + register_it + iterator, "K-N" + std::to_string(iterator + 1) + std::to_string(_bottle.second.id), "Kaskade " + std::to_string(iterator + 1) + " Seriennr. Gasflasche " + std::to_string(_bottle.second.id), {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_getBottleSerial, .ptr_arg = std::to_string(_bottle.second.id)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
                this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 2301 + register_it + iterator, "K-V" + std::to_string(iterator + 1) + std::to_string(_bottle.second.id), "Kaskade " + std::to_string(iterator + 1) + " Volumen Gasflasche " + std::to_string(_bottle.second.id), {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_getBottleVolume0, .ptr_arg = std::to_string(_bottle.second.id)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
                this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 2401 + register_it + iterator, "K-VA" + std::to_string(iterator + 1) + std::to_string(_bottle.second.id), "Kaskade " + std::to_string(iterator + 1) + " Volumenausdehnung Gasflasche " + std::to_string(_bottle.second.id), {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_getBottleVolumeRef, .ptr_arg = std::to_string(_bottle.second.id)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
                this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 2501 + register_it + iterator, "K-VAp" + std::to_string(iterator + 1) + std::to_string(_bottle.second.id), "Kaskade " + std::to_string(iterator + 1) + " Druck Volumenausdehnung Gasflasche " + std::to_string(_bottle.second.id), {.type = mb::INT32, .type_size = 4, .ptr = PlcInterface::mb_getBottlePressureRef, .ptr_arg = std::to_string(_bottle.second.id)}, USER, SERVICE, mb::AO, mb::BIDIRECTIONAL);
            }
        }

        register_it = register_it + 500;
    }

    // Fehlermeldungen
    register_it = 0;
    iterator = 0;
    for (const auto &_error: allErrors)
    {
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 14900 + register_it, "E" + std::to_string(iterator) + "-ID" , "Error " +  std::to_string(iterator) + " Id", {.type = mb::INT16, .type_size = 2, .ptr = PlcInterface::mb_getErrorId, .ptr_arg = std::to_string(_error.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 14901 + register_it, "E" + std::to_string(iterator) + "-CODE" , "Error " +  std::to_string(iterator) + " Code", {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getErrorCode, .ptr_arg = std::to_string(_error.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 14902 + register_it, "E" + std::to_string(iterator) + "-DERR" , "Error " +  std::to_string(iterator) + " Datum", {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getErrorDate, .ptr_arg = std::to_string(_error.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);  
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 14903 + register_it, "E" + std::to_string(iterator) + "-RES" , "Error " +  std::to_string(iterator) + " Gelöst", {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getErrorResolved, .ptr_arg = std::to_string(_error.second.id)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);
        
        register_it = register_it + 5;
        iterator++;
    }

    // Warnmeldungen
    register_it = 0;
    iterator = 0;
    for (const auto &_warning: allWarnings)
    {
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 15150 + register_it, "W" + std::to_string(iterator) + "-ID" , "Warning " +  std::to_string(iterator) + " Id", {.type = mb::INT16, .type_size = 2, .ptr = PlcInterface::mb_getWarningId, .ptr_arg = std::to_string(_warning.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 15151 + register_it, "W" + std::to_string(iterator) + "-CODE" , "Warning " +  std::to_string(iterator) + " Code", {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getWarningCode, .ptr_arg = std::to_string(_warning.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 15152 + register_it, "W" + std::to_string(iterator) + "-DWARN" , "Warning " +  std::to_string(iterator) + " Datum", {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getWarningDate, .ptr_arg = std::to_string(_warning.second.id)}, USER, READONLY, mb::AO, mb::OUTPUT);  
        this->dataMap[dataMapIndex++] = functionPtrToPlcData(reg, 15153 + register_it, "W" + std::to_string(iterator) + "-RES" , "Warning " +  std::to_string(iterator) + " Gelöst", {.type = mb::STRING, .type_size = 20, .ptr = PlcInterface::mb_getWarningResolved, .ptr_arg = std::to_string(_warning.second.id)}, USER, CALIBRATION, mb::AO, mb::BIDIRECTIONAL);  
        
        register_it = register_it + 5;
        iterator++;
    }

    for (const auto &item: this->dataMap) {
        this->registerMap[item.second.register_data.address] = item.first;
    }

    //std::cout << dumpRegisterMapping() << std::endl;
}


void PlcInterface::loadData(PlcInterface::usergroup accessClearance) {
    for (const auto &item: this->dataMap) {
        if (item.second.access_read >= accessClearance) {
            this->setDataStructToRegister(item.second.register_data);
            uint16_t nec_rec_size = item.second.register_data.address + item.second.register_data.total_register_length;

            switch (item.second.register_data.register_type) {
                case mb::DO:
                    if (this->getModbusConfig().do_reg_count < nec_rec_size)
                        this->getModbusConfig().do_reg_count = nec_rec_size;
                    break;
                case mb::DI:
                    if (this->getModbusConfig().di_reg_count < nec_rec_size)
                        this->getModbusConfig().di_reg_count = nec_rec_size;
                    break;
                case mb::AI:
                    if (this->getModbusConfig().ai_reg_count < nec_rec_size)
                        this->getModbusConfig().ai_reg_count = nec_rec_size;
                    break;
                case mb::AO:
                    if (this->getModbusConfig().ao_reg_count < nec_rec_size)
                        this->getModbusConfig().ao_reg_count = nec_rec_size;
                    break;
            }
        }
    }
}


void PlcInterface::unloadData(PlcInterface::usergroup accessClearance) {
    for (const auto &item: this->dataMap) {
        if (item.second.access_read == accessClearance) {
            if (item.second.access_read < accessClearance
                && item.second.access_write < accessClearance) {
                this->removeDataStructFromRegister(item.second.register_data);
            }
        }
    }
}

void PlcInterface::setAccessClearance(PlcInterface::usergroup accessClearanceRead, PlcInterface::usergroup accessClearanceWrite) {
    this->access_read = accessClearanceRead;
    this->access_write = accessClearanceWrite;
}

std::vector<uint16_t> PlcInterface::mb_receiveVesselData_Sensor(mb::modbus_register_data *data, const mb::telegram &t) {
    struct sensor _sensor;
    int reg_data_len = t.data_len / 2 + t.data_len % 2;
    std::vector<uint16_t> reg_data;
    for (int i = 0; i < reg_data_len; ++i) {
        reg_data.emplace_back(0x0000);
    }

    for (int i = 0; i < t.data.size(); ++i) {
        u_int8_t byte = t.data[i];
        if (i % 2 == 0) {
            reg_data[i / 2] = reg_data[i / 2] | byte;
        } else {
            reg_data[i / 2] = reg_data[i / 2] | (byte << 8);
        }
    }

    auto subVec = [] (
            std::vector<uint16_t> vector,
            int &at,
            int items
    ) -> std::vector<uint16_t> {
        std::vector<uint16_t> sub;
        for (int i = 0; i < items; ++i) {
            sub.emplace_back(vector[at + i]);
        }
        at += items;

        return sub;
    };


    auto bigToSysEndian = [] (
            std::vector<uint16_t> vector
    ) -> std::vector<uint16_t> {
        return mb::from_to_endian(vector, {true, true}, mb::determine_endianness());
    };


    int offset = 0;

    _sensor.id = registerDataToSimpleType<int> (bigToSysEndian(subVec(reg_data, offset, sizeof(int) / 2)));

    _sensor.type = registerDataToSimpleType<int> (bigToSysEndian(subVec(reg_data, offset, sizeof(int) / 2)));

    _sensor.type_order = registerDataToSimpleType<int> (bigToSysEndian(subVec(reg_data, offset, sizeof(int) / 2)));

    _sensor.serialnumber = registerDataToString (subVec(reg_data, offset, 10));

    _sensor.name = registerDataToString (subVec(reg_data, offset, 10));

    _sensor.manufacturer = registerDataToString (subVec(reg_data, offset, 10));

    _sensor.uppermeasurelimit_manufacturer = registerDataToSimpleType<float> (bigToSysEndian(subVec(reg_data, offset, sizeof(float ) / 2)));

    _sensor.lowermeasurelimit_manufacturer = registerDataToSimpleType<float> (bigToSysEndian(subVec(reg_data, offset, sizeof(float ) / 2)));

    _sensor.fk_hardwareprotocol = registerDataToSimpleType<int> (bigToSysEndian(subVec(reg_data, offset, sizeof(float ) / 2)));

    _sensor.hardwareprotocol_address = registerDataToSimpleType<int> (bigToSysEndian(subVec(reg_data, offset, sizeof(float ) / 2)));

    _sensor.offset = registerDataToSimpleType<float> (bigToSysEndian(subVec(reg_data, offset, sizeof(float ) / 2)));

    InitialSetupDataCache::add_sensor(_sensor);

    return reg_data;
}

std::vector<uint16_t> PlcInterface::mb_receiveVesselData_Cascade(mb::modbus_register_data *data, const mb::telegram &t) {
    struct cascade _cascade;

    int reg_data_len = t.data_len / 2 + t.data_len % 2;
    std::vector<uint16_t> reg_data;
    for (int i = 0; i < reg_data_len; ++i) {
        reg_data.emplace_back(0x0000);
    }

    for (int i = 0; i < t.data.size(); ++i) {
        u_int8_t byte = t.data[i];
        if (i % 2 == 0) {
            reg_data[i / 2] = reg_data[i / 2] | byte;
        } else {
            reg_data[i / 2] = reg_data[i / 2] | (byte << 8);
        }
    }

    auto subVec = [] (
            std::vector<uint16_t> vector,
            int &at,
            int items
    ) -> std::vector<uint16_t> {
        std::vector<uint16_t> sub;
        for (int i = 0; i < items; ++i) {
            sub.emplace_back(vector[at + i]);
        }
        at += items;

        return sub;
    };

    auto bigToSysEndian = [] (
            std::vector<uint16_t> vector
    ) -> std::vector<uint16_t> {
        return mb::from_to_endian(vector, {true, true}, mb::determine_endianness());
    };


    int offset = 0;

    _cascade.id = registerDataToSimpleType<int> (bigToSysEndian(subVec(reg_data, offset, sizeof(int) / 2)));
    _cascade.fk_pressure_sensor_upper = registerDataToSimpleType<int> (bigToSysEndian(subVec(reg_data, offset, sizeof(int) / 2)));
    _cascade.fk_pressure_sensor_lower = registerDataToSimpleType<int> (bigToSysEndian(subVec(reg_data, offset, sizeof(int) / 2)));

    InitialSetupDataCache::add_cascade(_cascade);

    return {reg_data.begin(), reg_data.end()};
}


std::vector<uint16_t> PlcInterface::mb_receiveVesselData_Bottle(mb::modbus_register_data *data, const mb::telegram &t) {
    struct bottle _bottle;

    int reg_data_len = t.data_len / 2 + t.data_len % 2;
    std::vector<uint16_t> reg_data;
    for (int i = 0; i < reg_data_len; ++i) {
        reg_data.emplace_back(0x0000);
    }

    for (int i = 0; i < t.data.size(); ++i) {
        u_int8_t byte = t.data[i];
        if (i % 2 == 0) {
            reg_data[i / 2] = reg_data[i / 2] | byte;
        } else {
            reg_data[i / 2] = reg_data[i / 2] | (byte << 8);
        }
    }

    auto subVec = [](
            std::vector<uint16_t> vector,
            int &at,
            int items
    ) -> std::vector<uint16_t> {
        std::vector<uint16_t> sub;
        for (int i = 0; i < items; ++i) {
            sub.emplace_back(vector[at + i]);
        }
        at += items;

        return sub;
    };

    auto bigToSysEndian = [](
            std::vector<uint16_t> vector
    ) -> std::vector<uint16_t> {
        return mb::from_to_endian(vector, {true, true}, mb::determine_endianness());
    };

    int offset = 0;

    _bottle.id = registerDataToSimpleType<int>(bigToSysEndian(subVec(reg_data, offset, sizeof(int) / 2)));

    _bottle.serialnumber = registerDataToString (subVec(reg_data, offset, 10));

    _bottle.fk_cascade = registerDataToSimpleType<int>(bigToSysEndian(subVec(reg_data, offset, sizeof(int) / 2)));

    _bottle.cascade_order = registerDataToSimpleType<int>(bigToSysEndian(subVec(reg_data, offset, sizeof(int) / 2)));

    _bottle.fk_sensor = registerDataToSimpleType<int>(bigToSysEndian(subVec(reg_data, offset, sizeof(int) / 2)));

    _bottle.tara = registerDataToSimpleType<float> (bigToSysEndian(subVec(reg_data, offset, sizeof(float ) / 2)));

    _bottle.manufacturer = registerDataToString (subVec(reg_data, offset, 10));

    _bottle.builtyear = registerDataToSimpleType<int>(bigToSysEndian(subVec(reg_data, offset, sizeof(int) / 2)));

    _bottle.nextcheck = registerDataToSimpleType<int>(bigToSysEndian(subVec(reg_data, offset, sizeof(int) / 2)));

    _bottle.vol_0 = registerDataToSimpleType<float> (bigToSysEndian(subVec(reg_data, offset, sizeof(float ) / 2)));

    _bottle.pressure_0 = registerDataToSimpleType<float> (bigToSysEndian(subVec(reg_data, offset, sizeof(float ) / 2)));

    _bottle.vol_ref = registerDataToSimpleType<float> (bigToSysEndian(subVec(reg_data, offset, sizeof(float ) / 2)));

    _bottle.pressure_ref = registerDataToSimpleType<float> (bigToSysEndian(subVec(reg_data, offset, sizeof(float ) / 2)));

    InitialSetupDataCache::add_bottle(_bottle);

    return {reg_data.begin(), reg_data.end()};
}

std::vector<uint16_t> PlcInterface::mb_receiveFirmwareUpdate(mb::modbus_register_data *data, const mb::telegram &t) {


    Core::instance->asyncHandler->setHandleActive(async::e_handle_plc_interface, false);
    Core::instance->asyncHandler->setHandleActive(async::e_handle_plc_firmware_update_via_uart, true);

    // Set handle_plc_interface job inactive
    // Set handle_plc_firmware_update_via_uart job actived

    return {(uint16_t) true};
}
