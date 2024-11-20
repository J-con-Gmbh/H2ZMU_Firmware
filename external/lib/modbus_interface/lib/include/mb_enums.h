//
// Created by jriessner on 27.02.23.
//

#ifndef MODBUS_INTERFACE_MB_ENUMS_H
#define MODBUS_INTERFACE_MB_ENUMS_H

namespace mb {

    /**
     * enum interface_type <br>
     * The interface type represents the currently used interface for serving data via the modbus protocol
     */
    enum interface_type {
        /// In memory serving, for test purposes in the same binary
        MEMORY = 0,
        /// Serial interface (Unix-Like: /dev/tty* | MS: COM*)
        SERIAL = 1,
        /// Virtual interface TODO some kind of virtual communication on the same system e.g. named pipe etc
        VIRTUAL = 2,
        /// Tcp interface
        TCP = 3,
    };

    /**
     * enum modbus_address_dest_type <br>
     * The Modbus address destination type describes the type of data which is to expect if a request is made via the modbus Protocol
     */
    enum modbus_address_dest_type {
        /// The Data is accessed in the Modbus instance by value. <br>Should be used for static data which is set initially
        VALUE = 0,
        /// The Data is accessed in the Modbus instance by reference. <br>Should be used for dynamic data with frequent change
        POINTER = 1,
        /// The Data is generated on-access by a linked function. <br>Should be used by real-time data, like system timestamp
        FUNCTION_POINTER = 2,
    };

    enum modbus_data_pointer_type {
        INT16 = 0,
        INT32 = 1,
        FLOAT32 = 2,
        BOOL = 3,
        STRING = 4,
        CHAR = 5,
    };

    /**
     * enum modbus_address_direction <br>
     * Specifies the direction of the data-flow of a modbus_register_data data-structure.
     */
    enum modbus_address_direction {
        /// Data should be set by Master
        INPUT = 0,
        /// Data should be read by Master
        OUTPUT = 1,
        /// No direction constrains
        BIDIRECTIONAL = 2,
    };

    /**
     * enum modbus_register_type <br>
     * Specifies register type
     */
    enum modbus_register_type {
        /// Discrete output coil; Type: Coil; Access: read\-write
        DO = 0,
        /// Discrete input coil; Type: Coil; Access: read
        DI = 10000,
        /// Analog input register; Type: Register; Access: read
        AI = 30000,
        /// Analog output holding registers; Type: Register; Access: read-write
        AO = 40000,
    };

}
#endif //MODBUS_INTERFACE_MB_ENUMS_H
