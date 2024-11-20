//
// Created by jriessner on 27.02.23.
//

#ifndef MODBUS_INTERFACE_MB_STRUCTS_H
#define MODBUS_INTERFACE_MB_STRUCTS_H

#include "mb_enums.h"
#include "mb_union_types.h"

namespace mb {
    struct modbus_register_data;
    struct telegram;

    typedef std::vector<uint16_t> (*mb_function_pointer)(modbus_register_data *data, const telegram & telegram);

    /**
     * struct modbus_data_pointer <br>
     * Holds information and destination about dynamic modbus data, which is loaded on access via a reference
     */
    struct modbus_data_pointer {
        mb::modbus_data_pointer_type type{};
        uint8_t type_size = 0;
        void *ptr = nullptr;
    };
    /**
     * struct modbus_function_pointer <br>
     * Holds information and destination about dynamic function, that generates modbus data, which is loaded on access via a reference
     */
    struct modbus_function_pointer {
        mb::modbus_data_pointer_type type{};
        uint8_t type_size = 0;
        mb_function_pointer ptr = nullptr;
        std::string ptr_arg;
    };

    /**
     * struct modbus_register_data <br>
     * Represents a data structure, which can hold data of <b>one or more</b> registers! <br>
     * Specifies all necessary information for accessing data of the given register-address.
     */
    struct modbus_register_data {
        mb::modbus_address_dest_type type = mb::VALUE;
        mb::modbus_register_type register_type = mb::DO;
        /// Direction of Dataflow
        mb::modbus_address_direction direction = mb::OUTPUT;
        /// Address of the data
        uint16_t address{};
        /// Length of the data
        uint16_t total_register_length = 0;
        /// <b>REQUIRED</b> for type String; Count of chars represented via Modbus if struct is type STRING
        uint16_t max_string_length = 0;
        /// If type == VALUE: Value of the Register, else: not used
        std::vector<uint16_t> value;
        /// If type == POINTER: Pointer to value of the Register, else: not used
        modbus_data_pointer data_pointer;
        /// If type == FUNCTION_POINTER: Pointer to function of the Register, else: not used
        modbus_function_pointer function_pointer;
    };

    struct modbus_interface_config {
        /// Connection, e.g. RTU: "/dev/ttyS0", TCP:"127.0.0.1"
        std::string connection_point;
        /// Baud-rate, necessary for Serial Connections. <br>Default 9600
        int serial_baud = 9600;
        /// Parity, necessary for Serial Connections. <br>Default 'N'
        char serial_parity = 'N';
        /// Data bit, necessary for Serial Connections. <br>Default 8
        uint8_t serial_data_bits = 8;
        /// Stop bit, necessary for Serial Connections. <br>Default 1
        uint8_t serial_stop_bit = 1;
        /// Network port, necessary for TCP Connections. <br>Default '127.0.0.1'
        uint16_t network_port = 502;
    };


    /**
     * struct modbus_interface <br>
     * The modbus interface struct holds the information about the currently used interface for serving data via the modbus protocol
     */
    struct modbus_interface {
        /// Interface type
        enum mb::interface_type type = mb::MEMORY;
        /// Interface config
        modbus_interface_config config{};
        /// Master Device
        bool master = false;
        /// Device-Id (Device-Address, Slave-Id)
        uint8_t s_id = 0;
        /// Debug mode toggle
        bool debug = false;
    };

    /**
     * struct endianness <br>
     * holds information about the endianness and byte order
     */
    struct endianness {
    private:
        struct big_end {
            bool *a;
            bool *b;

            explicit operator bool() const { return *a && *b; }
        };

        struct little_end {
            bool *a;
            bool *b;

            explicit operator bool() const { return ((!*a) && (!*b)); }
        };

    public:
        bool high_word_first = false;
        bool high_byte_first = false;

        big_end big_endian{&this->high_word_first, &this->high_byte_first};
        little_end little_endian{&this->high_word_first, &this->high_byte_first};
    };


    /**
     * struct modbus_config <br>
     * The modbus config holds all information about the current modbus instance,
     * as like the used interface, interface configuration, address mapping and much more.<br>
     */
    struct modbus_config {
        struct modbus_interface interface;
        /// Modbus Address 1 - 255 (Slave Id)
        uint8_t mb_id = 1;
        /// determine if modbus address range is e.g. 40001-49999 or 40000-49998. <br>Only difference is a offset in internal memory.
        bool address_zero_based = false;
        std::map<uint16_t, modbus_register_data> addressDestLookup;

        /// default is big endian
        endianness endian;

        /// Timeout for async calls (seconds, default = 0)
        uint32_t async_timeout_sec = 0;
        /// Timeout for async calls (microseconds, default = 100)
        uint32_t async_timeout_usec = 100;

        uint16_t do_reg_count = 10;
        uint16_t di_reg_count = 10;
        uint16_t ao_reg_count = 10;
        uint16_t ai_reg_count = 10;
    };


    struct telegram {
        uint8_t function_code = 0x00;
        uint8_t bytes_transmitted = 0;
        u_int16_t reg_start = 0;
        u_int16_t reg_count = 0;
        u_int16_t reg_offset = 0;
        uint16_8 reg_transmitted{};
        std::vector<uint8_t> data;
        uint8_t data_len = 0;
        uint8_t crc[2];
        uint8_t req_len = 0;
        std::vector<uint8_t> req;
    };
}

#endif //MODBUS_INTERFACE_MB_STRUCTS_H
