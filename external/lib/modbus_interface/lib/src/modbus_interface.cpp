#include <cstring>
#include <utility>
#include <iostream>
#include <fcntl.h>
#include <asm-generic/ioctls.h>
#include <sys/ioctl.h>
#include <sstream>
#include <iomanip>

#include "modbus_interface.h"

namespace mb {


    std::vector<uint16_t> reverse_words(std::vector<uint16_t> data) {

    	auto it_start = data.rbegin();
    	auto it_end = data.rend();
    	std::vector<uint16_t> ret;

    	for(auto it = it_start; it != it_end; ++it) {
    		ret.emplace_back(*it);
    	}

        //std::reverse(data.begin(), data.end());


        return data;
    }

    std::vector<uint16_t> switch_bytes(std::vector<uint16_t> data) {
        for (unsigned short & i : data) {
            uint16_8 uint168 { .value = i };
            uint16_8 to_data{};

            // reverse byte order
            to_data.split[0] = uint168.split[1];
            to_data.split[1] = uint168.split[0];
            i = to_data.value;
        }
        return data;
    }

    std::vector<uint16_t> switch_endian(const std::vector<uint16_t>& data) {

        return reverse_words(switch_bytes(data));
    }

    std::vector<uint16_t> to_big_endian(const std::vector<uint16_t>& data) {

        struct endianness endian = determine_endianness();

        return from_to_endian(data, endian, {true, true});
    }

    struct endianness determine_endianness()  {

        // 0x4015c28f -> big endian
        // 0x15408fc2 -> high word first, low byte first
        // 0xc28f4015 -> low word first, high byte first
        // 0x8fc21540 -> little endian

        union fv {
            float f;
            uint16_t words[2];
            uint8_t bytes[4];
        };

        fv float_union;
        float_union.f = 2.34;

        struct endianness ret { false, false };

        switch (float_union.bytes[0]) {
            case 0x40:
            {
                // Big endian
                ret.high_word_first = true;
                ret.high_byte_first = true;
            } break;
            case 0x15:
            {
                // high word first, low byte first
                ret.high_word_first = true;
                ret.high_byte_first = false;
            } break;
            case 0xc2:
            {
                // low word first, high byte first
                ret.high_word_first = false;
                ret.high_byte_first = true;
            } break;
            case 0x8f:
            {
                // little endian
                ret.high_word_first = false;
                ret.high_byte_first = false;
            } break;
        }

        return ret;
    }

    std::vector<uint16_t> from_to_endian(const std::vector<uint16_t>& data, endianness from, endianness to) {
        std::vector<uint16_t> dest = data;
        if (from.high_word_first != to.high_word_first) {
            dest = reverse_words(dest);
        }
        if (from.high_byte_first != to.high_byte_first) {
            dest = switch_bytes(dest);
        }
        return dest;
    }

    std::vector<uint16_t> bytes_to_words(const std::vector<uint8_t> &data, endianness from, endianness to) {

        std::vector<uint16_t> rawData;
        uint16_8 uint168{};
        for (int i = 0; i < data.size(); ++i) {
            uint168.split[!((bool)(i % 2))] = data[i];
            if (i%2 == 1) {
                rawData.emplace_back(uint168.value);
                uint168.value = 0x0000;
            }
        }
        rawData = from_to_endian(rawData, from, to);

        return rawData;
    }

    void set_data_to_mb_ptr(modbus_data_pointer data_pointer, std::vector<uint16_t> raw_data) {

        switch (data_pointer.type) {

            case INT16: {
                auto val = (int16_t) raw_data[0];
                *(int16_t *) data_pointer.ptr = val;
            } break;

            case INT32: {
                reg_int32 regInt32{};
                regInt32.data[0] = raw_data[1];
                regInt32.data[1] = raw_data[0];
                *(int *) data_pointer.ptr = regInt32.value;
                break;
            }

            case FLOAT32: {
                reg_float32 regFloat32{};
                regFloat32.data[0] = raw_data[1];
                regFloat32.data[1] = raw_data[0];
                *(float *) data_pointer.ptr = regFloat32.value;
                break;
            }

            case BOOL: {
                reg_bool regBool{};
                regBool.data[0] = raw_data[0];
                *(bool *) data_pointer.ptr = regBool.value;
                break;
            }

            case STRING: {
                std::string str = register_data_to_string(raw_data);
                *(std::string *) data_pointer.ptr = str;
                break;
            }

            case CHAR:
                break;
        }

    }

    std::string register_data_to_string(const std::vector<uint16_t> &data) {
        std::string ret;
        union word_byte {
            uint16_t value = 0;
            uint8_t split[2];
        };
        for (const auto &item: data) {
            word_byte wb;
            wb.value = item;
            if (wb.split[0] != 0x03) {
                ret += (unsigned char) wb.split[0];
            } else { break;}

            if (wb.split[1] != 0x03) {
                ret += (unsigned char) wb.split[1];
            } else { break;}
        }

        return ret;
    }

    ModbusServer::ModbusServer(const struct modbus_config &config) {
        this->modbusConfig = config;

        setupInterface = nullptr;
        closeInterface = nullptr;
        receiveFromInterface = nullptr;
        sendToInterface = nullptr;

        this->request = (uint8_t*) malloc(MB_REQUEST_LENGTH);

    }

    uint16_t ModbusServer::setDataStructToRegister(modbus_register_data data) {

        if (data.total_register_length == 0) {
            if (data.type == mb::VALUE) {
                int reg_len = data.value.size() / 2;
                data.total_register_length = reg_len;
            } else if (data.type == mb::POINTER) {
                switch (data.data_pointer.type) {
                    case INT16:
                        data.data_pointer.type_size = sizeof(int16_t);
                        break;
                    case mb::INT32:
                        data.data_pointer.type_size = sizeof(int);
                        break;
                    case mb::FLOAT32:
                        data.data_pointer.type_size = sizeof(float);
                        break;
                    case mb::BOOL:
                        data.data_pointer.type_size = 1;
                        break;
                    case mb::STRING:
                        data.data_pointer.type_size = data.max_string_length;
                        break;
                }
                if (!data.data_pointer.ptr) {
                    data.data_pointer.ptr = malloc(data.data_pointer.type_size);
                }
                data.total_register_length = (data.data_pointer.type_size / 2) + (data.data_pointer.type_size % 2);

            } else if (data.type == mb::FUNCTION_POINTER) {
                data.total_register_length = data.function_pointer.type_size;
            }
        } else if (data.data_pointer.type != mb::STRING) {
            // TODO implement Error
            // Only Strings need max-length predefined
        }

        if (data.address > 10000 && data.register_type == 1) {
            data.register_type = (mb::modbus_register_type) ( ( (data.address / 10000) * 10000) + 1);
        }

        // Test for double address assignment
        if (this->modbusConfig.addressDestLookup.count(data.address)) {
            std::cerr << "Double address assignment!" << std::endl;
            return -1;
        }
        for (int i = 0; i < data.total_register_length; ++i) {
            if (this->addressData.count(data.address + i)) {
                std::cerr << "Double address assignment!" << std::endl;
                return -1;
            }
        }

        this->modbusConfig.addressDestLookup[data.address] = data;
        for (int i = 0; i < data.total_register_length; ++i) {
            this->addressData[data.address + i] = data.address;
        }

        return data.data_pointer.type_size;
    }

    std::vector<uint16_t> ModbusServer::getDataFromMultipleRegister(uint16_t address, uint16_t register_count) {

        std::vector<uint16_t> reg_data(register_count);
        std::vector<uint16_t> ret(register_count);

        int iter = 0;
        while (iter < register_count) {
            modbus_register_data data = getDataStructFromRegister(address);
            std::vector<uint16_t> tmp = getRegisterDataFromDataStruct(data);

            int offset_tmp = (data.address < address) ? address - data.address : 0;
            int offset_ret = (data.address > address) ? data.address - address : 0;

            for (int i = 0; i < data.total_register_length; ++i) {
                if ((i + offset_ret) >= register_count) {
                    break;
                }
                ret[i + offset_ret] = tmp[i + offset_tmp];
            }

            iter += data.total_register_length;
        }

        return ret;
    }

    uint16_t ModbusServer::getDataFromSingleRegister(uint16_t address) {
        if (!this->addressData.count(address)) {
            return 0x0000;
        }
        modbus_register_data data = getDataStructFromRegister(address);
        std::vector<uint16_t> tmp(data.total_register_length);
        switch (data.type) {
            case mb::VALUE:
                tmp = data.value;
                break;
            case mb::POINTER:
                tmp = getDataFromPointer(data);
            default:
                break;
        }
        uint16_t offset = address - data.address;

        return tmp[offset];
    }

    std::vector<uint16_t> ModbusServer::getDataFromPointer(const modbus_register_data& data) {
        std::vector<uint16_t> ret(data.total_register_length);

        if (data.type != mb::POINTER) {
            return ret;
        }

        for (int i = 0; i < data.total_register_length; ++i) {

            modbus_data_pointer dataPointer = data.data_pointer;
            reg_int32 regInt32{};
            reg_float32 regFloat32{};
            reg_bool regBool{};
            reg_unsigned_chars regString{};
            std::string tmp;
            switch (dataPointer.type) {
                case mb::INT16:
                    ret[i] = *(uint16_t*) dataPointer.ptr;
                    break;
                case mb::INT32:
                    regInt32.value = *(int*) dataPointer.ptr;
                    ret[i] = regInt32.data[i];
                    break;
                case mb::FLOAT32:
                    regFloat32.value = *(float*) dataPointer.ptr;
                    ret[i] = regFloat32.data[i];
                    break;
                case mb::BOOL:
                    regBool.value = *(bool*) dataPointer.ptr;
                    ret[i] = regBool.data[i];
                    break;
                case mb::STRING:
                    tmp = *(std::string*) dataPointer.ptr;
                    for (int j = tmp.size(); j < data.max_string_length; ++j) {
                        tmp += (char) 0x00;
                    }
                    auto c_tmp = tmp.c_str();
                    size_t len = (data.max_string_length > 0) ? data.max_string_length : strlen(c_tmp);
                    regString.value[1] = ((len) > i*2) ? c_tmp[i*2] : 0x03;
                    regString.value[0] = ((len) > ((i*2)+1)) ? c_tmp[(i*2)+1] : 0x03;
                    ret[i] = regString.data;
                    break;
            }
        }

        return ret;
    }

    std::vector<uint16_t> ModbusServer::getDataFromFunctionPointer(modbus_register_data data, const telegram &t) {
        std::vector<uint16_t> ret(data.total_register_length);

        if (data.type != mb::FUNCTION_POINTER) {
            return ret;
        }

        auto tmp = data.function_pointer.ptr(&data, t);
        for (int i = 0; (i < ret.size()) && (i < tmp.size()); ++i) {
            ret[i] = tmp[i];
        }

        return ret;
    }

    modbus_register_data ModbusServer::getDataStructFromRegister(uint16_t address) {
        if (!this->addressData.count(address)) {
            return {};
        }
        modbus_register_data data(this->modbusConfig.addressDestLookup[this->addressData[address]]);

        return data;
    }

    std::vector<uint16_t> ModbusServer::getRegisterDataFromDataStruct(const modbus_register_data& data) {
        std::vector<uint16_t> ret(data.total_register_length);
        switch (data.type) {
            case mb::VALUE:
                ret = data.value;
                break;
            case POINTER:
                ret = getDataFromPointer(data);
                break;
            case FUNCTION_POINTER:
                ret = getDataFromFunctionPointer(data);
                break;
            default:
                break;
        }

        return ret;
    }

    std::vector<uint16_t> ModbusServer::stringToRegisterData(const std::string& str, int maxLen, int minLen, endianness ends) {
        uint16_t str_len = str.length();
        uint16_t len = getRegisterLengthForString(str);


        int target_min_len = ((minLen / 2) + (minLen % 2));
        int target_max_len = ((maxLen / 2) + (maxLen % 2));

        if (len > target_max_len) {
            len = target_max_len;
        }

        std::vector<uint16_t> ret;
        ret.reserve(len);

        for (int i = 0; i < len; ++i) {
            reg_unsigned_chars chars{};
            uint16_t index = i*2;

            if (index < str_len) {
                chars.value[1] = str.at(index);
            } else {
                chars.value[1] = 0x03;
                ret.emplace_back(chars.data);

                break;
            }
            if ((index + 1) < str_len) {
                chars.value[0] = str.at(index + 1);
            } else {
                chars.value[0] = 0x03;
                ret.emplace_back(chars.data);

                break;
            }
            ret.emplace_back(chars.data);
        }
        if (len < target_min_len ) {
            int diff = target_min_len - len;
            for (int i = 0; i < diff; ++i) {
                ret.emplace_back((char) 0x00);
            }
        }

        if (!ends.high_word_first) {
            ret = reverse_words(ret);
        }
        if (!ends.high_byte_first) {
            ret = switch_bytes(ret);
        }

        return ret;
    }

    std::vector<uint16_t> ModbusServer::int64ToRegisterData(long value) {
        reg_int64 ret{.value=value};
        uint16_t vector_len = sizeof ret.data / sizeof ret.data[0];
        std::vector<uint16_t> v(vector_len);
        for (int i = 0; i < vector_len; ++i) {
            v[i] = ret.data[vector_len - i - 1];
        }

        return v;
    }

    std::vector<uint16_t> ModbusServer::int32ToRegisterData(int value) {
        reg_int32 ret{.value=value};
        uint16_t vector_len = sizeof ret.data / sizeof ret.data[0];
        std::vector<uint16_t> v(vector_len);
        for (int i = 0; i < vector_len; ++i) {
            v[i] = ret.data[vector_len - i - 1];
        }

        return v;
    }

    std::vector<uint16_t> ModbusServer::floatToRegisterData(float value) {
        reg_float32 ret{.value=value};
        int vector_len = sizeof ret.data / sizeof ret.data[0];
        std::vector<uint16_t> v(vector_len);
        for (int i = 0; i < vector_len; ++i) {
            v[i] = ret.data[(vector_len - i) - 1];
        }

        return v;
    }

    std::vector<uint16_t> ModbusServer::boolToRegisterData(bool value) {
        reg_bool ret{.value=value};
        uint16_t vector_len = sizeof ret.data / sizeof ret.data[0];
        std::vector<uint16_t> v(vector_len);
        for (int i = 0; i < vector_len; ++i) {
            v[i] = ret.data[vector_len - i - 1];
        }

        return v;
    }

    uint16_t ModbusServer::getRegisterLengthForString(const std::string& str) {
        uint16_t str_len = str.length();
        uint16_t len = 0;
        // one additional char for the ETX byte is needed
        // every register can store two chars
        if ( (str_len % 2) == 0) {
            // if length is even, we need an additional register for ETX
            len = (str_len / 2) + 1;
        } else {
            // if length is uneven, we can store ETX in the second byte of the last register
            // str_len needs to be the next higher even number for the division to put out the right register length
            len = (str_len + 1) / 2;
        }

        return len;
    }

    void ModbusServer::setup() {

        if (setupComplete) {
            fprintf(stderr, "Multiple registerAsyncJobs of same ModbusServer instance\n");
            return;
        }

        this->ctx = init_context(
                this->modbusConfig.mb_id,
                this->modbusConfig.do_reg_count,
                this->modbusConfig.di_reg_count,
                this->modbusConfig.ao_reg_count,
                this->modbusConfig.ai_reg_count
                );

        writeDataToLibmodbusRegister();

        bool success = true;
        if (this->setupInterface != nullptr) {
            success = this->setupInterface();
        }

        this->active = success;

        setupComplete = success;
    }

    void ModbusServer::connect() {
        if (this->connected) {
            return;
        }

        std::cout << "mb connect" << std::endl;
        this->connected = true;
    }

    void ModbusServer::disconnect() {
        if (!this->connected) {
            return;
        }

        std::cout << "mb disconnect" << std::endl;
        if (this->active) {
            setInactive();
        }
        free_context(ctx);

        this->connected = false;
    }

    void ModbusServer::serve() {
        std::cout << "mb serve" << std::endl;
        this->active = true;
        this->serving = true;

        int rc = 0;
        while (this->active) {
            do {
                rc = this->receiveFromInterface(request, MB_REQUEST_LENGTH);
                /* Filtered queries return 0 */
            } while (rc == 0);

            telegram t{};
            mb_telegram mb_t{};
            if (rc > 0) {
                mb_t = parse_telegram(request, rc);
                t = parseRequest(request, rc);
                processModbusTelegram(t);
            }

            mb_telegram t_reply = reply(ctx, mb_t);

            rc = this->sendToInterface(t_reply.payload, t_reply.size);

            if (rc == -1) {
                std::cout << "mb rc -1" << std::endl;
                break;
            }
        }

        this->serving = false;
    }

    void ModbusServer::serveNonBlocking() {

        if ( !(this->active && this->serving) ) {
            return;
        }

        fd_set rfds;
        struct timeval tv{};

        FD_ZERO(&rfds);
        FD_SET(this->fd, &rfds);

        tv.tv_sec = this->modbusConfig.async_timeout_sec;
        tv.tv_usec = this->modbusConfig.async_timeout_usec;

        int av = select(this->fd + 1, &rfds, nullptr, nullptr, &tv);
        if (av < 0) {
            std::cerr << "Err " << av << " select();" << std::endl;
            return;
        }
        if (av == 0) {
            return;
        }

        int rc;
        rc = this->receiveFromInterface(request, MB_REQUEST_LENGTH);

        if (rc == 0){
            return;
        }

        telegram t{};
        mb_telegram t_request{};

        if (rc > 0) {
            mb_telegram t_request = parse_telegram(request, rc);
            t = parseRequest(request, rc);
            processModbusTelegram(t);
        } else {
            return;
        }

        mb_telegram t_reply = reply(ctx, t_request);
        bool success = this->sendToInterface(t_reply.payload, t_reply.size);

        if (!success) {
            std::cout << "mb rc -1" << std::endl;
            return;
        }
    }

    void ModbusServer::setInactive() {
        this->active = false;
    }

    void ModbusServer::quit() {
        free_context(ctx);
        free(request);

        bool success = true;
        if (this->closeInterface != nullptr) {
            success = this->closeInterface();
        }

        this->active = !success;
    }

    telegram ModbusServer::parseRequest(const uint8_t *req, int len) {
        uint8_t fn_code = req[1];

        int i;
        uint16_8 reg_s{};
        reg_s.split[1] = req[2];
        reg_s.split[0] = req[3];

        int offset = 6;

        uint16_t reg_offset;
        switch (fn_code) {
            case 0x01:
                reg_offset = DO;
                break;
            case 0x02:
                reg_offset = DI;
                break;
            case 0x03:
                reg_offset = AO;
                break;
            case 0x04:
                reg_offset = AI;
                break;
            case 0x05:
                offset = 4;
                reg_offset = DO;
                break;
            case 0x06:
                offset = 4;
                reg_offset = AO;
                break;
            case 0x0F:
                reg_offset = DO;
                break;
            case 0x10:
                offset = 7;
                reg_offset = AO;
                break;
            default:
                break;
        }
        uint8_t dataLen = len - ( offset + 2 );
        uint16_t regLen = 1;
        if (fn_code < 0x05 || 0x06 < fn_code) {
            regLen = 0x0000 | (req[4] << 8 | req[5]);
        }

        std::vector<uint8_t> reqV;
        std::vector<uint8_t> dataV;
        for (int j = 0; j < len; ++j) {
            reqV.push_back(req[j]);
            if ( ( j >= offset ) && ( j < (len - 2) ) ) {
                dataV.push_back(req[j]);
            }
        }

        telegram t{
                .function_code = fn_code,
                .reg_start = reg_s.value,
                .reg_count = regLen,
                .reg_offset = reg_offset,
                .data = dataV,
                .data_len = dataLen,
                .crc = {req[len-2], req[len-1]},
                .req_len = (uint8_t) len,
                .req = reqV,
        };

        for (int j = offset; j < len - 2; ++j) {
            t.data[j - offset] = req[j];
        }

        return t;
    }

    void ModbusServer::writeDataToLibmodbusRegister() {

        for (const auto &item: this->modbusConfig.addressDestLookup) {
            uint8_t reg_len = item.second.total_register_length;
            uint16_t abs_addr = item.second.address % 10000;
            switch (item.second.register_type) {
                case DO:
                    if (abs_addr + reg_len > this->modbusConfig.do_reg_count) {
                        std::cerr << "ERROR: not enough memory for DO registers allocated" << std::endl;
                        exit(1);
                    }
                    break;
                case DI:
                    if (abs_addr + reg_len > this->modbusConfig.di_reg_count) {
                        std::cerr << "ERROR: not enough memory for DI registers allocated" << std::endl;
                        exit(1);
                    }
                    break;
                case AO:
                    if (abs_addr + reg_len > this->modbusConfig.ao_reg_count) {
                        std::cerr << "ERROR: not enough memory for AO registers allocated" << std::endl;
                        exit(1);
                    }
                    break;
                case AI:
                    if (abs_addr + reg_len > this->modbusConfig.ai_reg_count) {
                        std::cerr << "ERROR: not enough memory for AI registers allocated" << std::endl;
                        exit(1);
                    }
                    break;
            }

            if (item.second.type != VALUE) {
                continue;
            }

            int index = item.second.address % 10000;
            if (!this->modbusConfig.address_zero_based) index -= 1;

            for (int i = 0; i < reg_len; ++i) {
                switch (item.second.register_type) {
                    case DO:
                        if (index + i > this->ctx.count_do) {
                            break;
                        }
                        this->ctx.reg_do[index] = item.second.value[i];
                        break;
                    case DI:
                        if (index + i > this->ctx.count_di) {
                            break;
                        }
                        this->ctx.reg_di[index] = item.second.value[i];
                        break;
                    case AI:
                        if (index + i > this->ctx.count_ai) {
                            break;
                        }
                        this->ctx.reg_di[index + i] = item.second.value[i];
                        break;
                    case AO:
                        if (index + i > this->ctx.count_ao) {
                            break;
                        }
                        this->ctx.reg_ao[index + i] = item.second.value[i];
                        break;
                }
            }

        }
    }

    void ModbusServer::writeDataToLibmodbusRegister(std::vector<uint16_t> reg_data, const modbus_register_data& data_struct) {
        uint8_t reg_len = data_struct.total_register_length;

        int index = data_struct.address % 10000;
        if (!this->modbusConfig.address_zero_based) index -= 1;

        for (int i = 0; (i < reg_len) && (i < reg_data.size()); ++i) {
            switch (data_struct.register_type) {
                case DO:
                    if (index + i > this->ctx.count_do) {
                        break;
                    }
                    this->ctx.reg_do[index] = reg_data[i];
                    break;
                case DI:
                    if (index + i > this->ctx.count_di) {
                        break;
                    }
                    this->ctx.reg_di[index] = reg_data[i];
                    break;
                case AI:
                    if (index + i > this->ctx.count_ai) {
                        break;
                    }
                    this->ctx.reg_di[index + i] = reg_data[i];
                    break;
                case AO:
                    if (index + i > this->ctx.count_ao) {
                        break;
                    }
                    this->ctx.reg_ao[index + i] = reg_data[i];
                    break;
            }
        }

    }

    void ModbusServer::printRegisterMapping() {
        for (const auto &item: this->modbusConfig.addressDestLookup) {
            std::cout << item.second.address << ":\t" << "Type ";
            std::cout << ((item.second.direction == INPUT) ? "INPUT " : "OUTPUT ");
            switch (item.second.type) {
                case VALUE:
                    std::cout << "VALUE ";
                    std::cout << "0x" << std::hex << item.second.value[0] << item.second.value[1] << std::dec;
                    break;
                case POINTER:
                    std::cout << "POINTER";
                    break;
                case FUNCTION_POINTER:
                    std::cout << "FUNCTION_POINTER";
                    break;
            }
            std::cout << "\t" << item.second.total_register_length << " register" << std::endl;
        }
    }

    void ModbusServer::processModbusTelegram(telegram t) {

        int regStart = t.reg_offset + t.reg_start;
        uint16_t lastRegisterDataIndex = 0;

        // check all register that are affected by this telegram
        for (int index = regStart; index < (regStart + t.reg_count); ++index) {

            /// Continue if no data struct represents data in given Register,
            /// or the data struct is already processed
            if (!this->addressData.count(index)) {
                continue;
            }

            mb::modbus_register_data registerData = this->modbusConfig.addressDestLookup[this->addressData[index]];

            /// Continue if data struct is set by Value -> data is present in libmodbus since call of function ModbusServer::serve()
            if (registerData.type == VALUE) {
                continue;
            }

            /// If modbus function is read access
            if ( t.function_code <= 0x04 ) {
                if (registerData.type == POINTER) {
                    writeDataToLibmodbusRegister(getDataFromPointer(registerData), registerData);
                } else if (registerData.type == FUNCTION_POINTER) {
                    writeDataToLibmodbusRegister(getDataFromFunctionPointer(registerData, t), registerData);
                }

            } else if ( registerData.direction == INPUT
                        || registerData.direction == BIDIRECTIONAL ) {

                std::vector<uint16_t> rawData = bytes_to_words(t.data);

                if (registerData.type == POINTER) {
                    set_data_to_mb_ptr(registerData.data_pointer, rawData);
                } else if (registerData.type == FUNCTION_POINTER) {
                    std::vector<uint16_t> ret = registerData.function_pointer.ptr(&registerData, t);
                    if (registerData.direction == BIDIRECTIONAL) {
                        writeDataToLibmodbusRegister(ret, registerData);
                    }
                }
            }

            index += (registerData.total_register_length - 1);
        }

    }

    bool ModbusServer::removeDataStructFromRegister(modbus_register_data data) {

        if (!this->modbusConfig.addressDestLookup.count(data.address)) {
            return false;
        }

        auto toDelete = this->modbusConfig.addressDestLookup[data.address];
        /// Test if the saved dataset is really the one we want to delete
        if ( !(     toDelete.total_register_length != data.total_register_length
                    || toDelete.type != data.type
                    || toDelete.address != data.address
        )
                ) {
            return false;
        }

        this->modbusConfig.addressDestLookup.erase(data.address);

        for (int i = 0; i < data.total_register_length; ++i) {
            if (this->addressData.count(data.address + i)) {
                this->addressData.erase(data.address + i);
            } else {
                std::cerr << "Address data inconsistency at data removal!" << std::endl;
                return false;
            }
        }
        /// Clear out cached values in libmodbus mb_mapping.
        writeDataToLibmodbusRegister(std::vector<uint16_t>(data.total_register_length, 0x0000), data);

        return true;
    }

    void ModbusServer::setActiveNonBlocking() {
        this->active = true;
        this->serving = true;
    }

    void ModbusServer::setInactiveNonBlocking() {
        this->active = false;
        this->serving = false;
    }

    modbus_config &ModbusServer::getModbusConfig() {
        return this->modbusConfig;
    }

    void ModbusServer::setFd(int fd) {
        this->fd = fd;
    }

    bool ModbusClient::setup(const std::string& fd, bool debug = false) {
        this->ctx = init_context(0,0,0,0,0);

        bool success = true;
        if (this->setupInterface != nullptr) {
            success = this->setupInterface();
        }

        this->active = success;

        return true;
    }

    float ModbusClient::readFloatFromRegister(uint16_t reg1, uint16_t reg2, bool bigEndianInWord, bool highByteFirst, bool highWordFirst) {
        /*
            uint16_8 reg1s{};
            uint16_8 reg2s{};
            // flip word order on highWordFirst
            reg1s.value = highWordFirst ? reg1 : reg2;
            reg2s.value = highWordFirst ? reg2 : reg1;
            float32_int8 float32Int8{};

            //                change byte order if is big endian                flip byte order on highByteFirst
            float32Int8.split[(3 * bigEndianInWord) + (2 * !bigEndianInWord)] = reg1s.split[!highByteFirst]; // highByteFirst ? 0 : 1
            float32Int8.split[(2 * bigEndianInWord) + (3 * !bigEndianInWord)] = reg1s.split[ highByteFirst]; // highByteFirst ? 1 : 0
            float32Int8.split[(1 * bigEndianInWord) + (0 * !bigEndianInWord)] = reg2s.split[!highByteFirst]; // highByteFirst ? 0 : 1
            float32Int8.split[(0 * bigEndianInWord) + (1 * !bigEndianInWord)] = reg2s.split[ highByteFirst]; // highByteFirst ? 1 : 0

            return float32Int8.value;
            */

        uint16_t dest[2];

        dest[0] = reg1;
        dest[1] = reg2;

        float eins;
        float zwei;
        float drei;
        float vier;

        return eins;
    }

#ifndef NDEBUG
    float ModbusClient::readFloatFromRegister_old(uint16_t reg1, uint16_t reg2) {
        uint16_8 reg1s{};
        uint16_8 reg2s{};
        reg1s.value = reg1;
        reg2s.value = reg2;
        float32_int8 float32Int8{};

        float32Int8.split[3] = reg1s.split[0];
        float32Int8.split[2] = reg1s.split[1];
        float32Int8.split[1] = reg2s.split[0];
        float32Int8.split[0] = reg2s.split[1];

        return float32Int8.value;
    }
#endif

    std::vector<uint16_t> ModbusClient::readInputRegisters(int startReg, int regCount) {
        uint16_t dest[regCount];
        std::vector<uint16_t> ret(regCount, 0);
        /*
        int num = modbus_read_input_registers(ctx, startReg, regCount, dest);
        if (num < 0) {
            std::stringstream msg;
            msg << "Fehler! num: " << num << " | regStart: " << startReg << " | regCount: " << regCount;
            std::cerr << msg.str() << std::endl;
        }
        for (int i = 0; i < num; ++i) {
            ret[i] = dest[i];
        }
        */
        return ret;
    }

    std::vector<uint16_t> ModbusClient::readHoldingRegisters(int startReg, int regCount) {
        uint16_t dest[regCount];
        std::vector<uint16_t> ret(regCount, 0);
        /*
        int num = modbus_read_registers(ctx, startReg, regCount, dest);
        if (num < 0) {
            std::stringstream msg;
            msg << "Fehler! num: " << num << " | regStart: " << startReg << " | regCount: " << regCount;
            std::cerr << msg.str() << std::endl;
        }
        for (int i = 0; i < num; ++i) {
            ret[i] = dest[i];
        }
        */
        return ret;
    }

    ModbusClient::~ModbusClient() {
        /*
        if (this->ctx) {
            modbus_close(this->ctx);
            modbus_free(this->ctx);
        }
        if (this->mb_mapping) {
            modbus_mapping_free(this->mb_mapping);
        }
         */
    }

    bool ModbusClient::isActive() {
        return this->active;
    }

    void ModbusClient::setServerId(int) {
        this->id;
    }

    ModbusClient::ModbusClient() {
        setupInterface = nullptr;
        closeInterface = nullptr;
        receiveFromInterface = nullptr;
        sendToInterface = nullptr;
    }

}
