#ifndef MODBUS_INTERFACE_MODBUS_INTERFACE_H
#define MODBUS_INTERFACE_MODBUS_INTERFACE_H

#include <string>
#include <map>
#include <functional>
#include <vector>

#include "mb_structs.h"
#include "modbus_pnp.h"

#define MB_REQUEST_LENGTH 256

// TODO Modbus Server und Client separieren
namespace mb {
    /**
     * This function switches the bytes in every word. <br>
     * The word order is not changed. <br>
     * example input 0x4015 0xc28f <br>
     * example output 0xc28f 0x4015
     * @param data
     * @return converted data
     */
    std::vector<uint16_t> reverse_words (std::vector<uint16_t> data);

    /**
     * This function switches the bytes in every word. <br>
     * The word order is not changed. <br>
     * example input 0x4015 0xc28f <br>
     * example output 0x1540 0x8fc2
     * @param data
     * @return converted data
     */
    std::vector<uint16_t> switch_bytes(std::vector<uint16_t> data);

    /**
     * This function completly reverses byte order <br>
     * example input 0x4015 0xc28f <br>
     * example output 0x8fc2 0x1540
     * @param data
     * @return reversed data
     */
    std::vector<uint16_t> switch_endian(const std::vector<uint16_t>& data);

    /**
     * Takes input data with the assumption, that the endianness of the data is like the function determine_endianness() evaluates. <br>
     * Converts the data to big endian
     * @param data
     * @return converted data
     */
    std::vector<uint16_t> to_big_endian(const std::vector<uint16_t>& data);

    /**
     * This function determines the system endianness for float values
     * @return endianness
     */
    struct endianness determine_endianness();

    /**
     * This function converts register data from the "from" endianness to the "to" endianness
     * @param data register data
     * @param from given endianness
     * @param to desired endianness
     * @return converted data
     */
    std::vector<uint16_t> from_to_endian(const std::vector<uint16_t>& data, endianness from, endianness to);

    /**
     * This function converts bytes to words (register data).
     * @param data bytes
     * @return converted data
     */
    std::vector<uint16_t> bytes_to_words(const std::vector<uint8_t>& data, endianness from = {true, true}, endianness to = {true, true});

    std::string register_data_to_string(const std::vector<uint16_t>& data);

    /**
     * This function converts bytes to words (register data).
     * @param data bytes
     */
    void set_data_to_mb_ptr(modbus_data_pointer data_pointer, std::vector<uint16_t> raw_data);

    typedef int (*receive_raw_data)(uint8_t* data_dest);
    typedef bool (*send_raw_data)(uint8_t* data_dest, int length);

    // Need to be set!!!
    typedef bool (*setup_interface)();
    typedef bool (*close_interface)();
    typedef int  (*receive_from_interface)(uint8_t*, int);
    typedef bool (*send_to_interface)(uint8_t*, int);

    // Former "Slave"
    class ModbusServer {
    private:
        int fd;
        bool setupComplete = false;
        bool active = true;
        bool connected = false;
        bool serving = false;

        modbus_config modbusConfig {};
        endianness systemEndianness = determine_endianness();

        std::map<uint16_t, uint16_t> addressData;
        std::map<uint16_t, uint16_t> cachedValues;

        /// Request data returned by libmodbus
        uint8_t *request;

        std::vector<uint16_t> getRegisterDataFromDataStruct(const modbus_register_data& data);
        static std::vector<uint16_t> getDataFromPointer(const modbus_register_data& data);
        static std::vector<uint16_t> getDataFromFunctionPointer(modbus_register_data data, const telegram &t = {});

        void writeDataToLibmodbusRegister();
        void writeDataToLibmodbusRegister(std::vector<uint16_t> reg_data, const modbus_register_data& data_struct);

    protected:
        /// libmodbus struct
        /// Modbus context
        mb_ctx ctx;

        virtual void processModbusTelegram(telegram t);

        modbus_config& getModbusConfig();

    public:
        // Need to be set!!!
        setup_interface setupInterface;
        close_interface closeInterface;
        receive_from_interface receiveFromInterface;
        send_to_interface sendToInterface;


        explicit ModbusServer(const struct modbus_config& config);
        uint16_t                setDataStructToRegister(modbus_register_data data);
        bool                    removeDataStructFromRegister(modbus_register_data data);
        modbus_register_data    getDataStructFromRegister(uint16_t address);
        uint16_t                getDataFromSingleRegister(uint16_t address);
        std::vector<uint16_t>   getDataFromMultipleRegister(uint16_t address, uint16_t register_count);

        virtual void setup();
        void setFd(int fd);
        void connect();
        void disconnect();
        void serve();
        void serveNonBlocking();
        void setActiveNonBlocking();
        void setInactiveNonBlocking();
        void setInactive();
        void quit();
        void printRegisterMapping();
        // TODO Async stop -> from different thread with waitung for (!this->serving)


        static telegram                 parseRequest(const uint8_t *req, int len);
        static std::vector<uint16_t>    stringToRegisterData(const std::string& str, int maxLen = 10, int minLen = 0, endianness ends = {true, true});
        static std::vector<uint16_t>    int64ToRegisterData(long value);
        static std::vector<uint16_t>    int32ToRegisterData(int value);
        static std::vector<uint16_t>    floatToRegisterData(float value);
        static std::vector<uint16_t>    boolToRegisterData(bool value);
        static uint16_t                 getRegisterLengthForString(const std::string& str);
    };


    // Former "Master"
    class ModbusClient {
    private:
        bool active = false;
        mb_ctx ctx;
        int id = 1;

    public:
        // Need to be set!!!
        setup_interface setupInterface;
        close_interface closeInterface;
        receive_from_interface receiveFromInterface;
        send_to_interface sendToInterface;

        ModbusClient();
        ~ModbusClient();
        virtual bool setup(const std::string& fd, bool debug);

        static float readFloatFromRegister(uint16_t reg1, uint16_t reg2, bool bigEndianInWord = true, bool highByteFirst = true, bool highWordFirst = true);
    #ifndef NDEBUG
        static float readFloatFromRegister_old(uint16_t reg1, uint16_t reg2);
    #endif
        bool isActive();
        void setServerId(int);

        std::vector<uint16_t> readInputRegisters(int startReg, int regCount);
        std::vector<uint16_t> readHoldingRegisters(int startReg, int regCount);

    };
}

#endif //MODBUS_INTERFACE_MODBUS_INTERFACE_H
