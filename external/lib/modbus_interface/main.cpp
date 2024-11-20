//
// Created by jriessner on 27.02.23.
//

#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <limits>

#include <thread>
#include <chrono>
#include <fcntl.h>
#include <cstring>
#include <csignal>

#include "lib/include/modbus_interface.h"

#define DEFAULT_ADDR "127.0.0.1"
#define DEFAULT_PORT 1502
#define DEFAULT_PROTOCOL 3 /// TCP -> mb_enums.h: enum interface_type
#define DEFAULT_CSV_PATH ".."
#define DEFAULT_CSV_FILE "testdata.csv"
#define DEBUG false

#define HELP_TEXT "Usage: modbus_test [OPTION]...\n   -i Interface type ( 0 = MEMORY, 1 = SERIAL, 2 = VIRTUAL, 3 = TCP, default TCP )\n   -a Interface connection Point ( Serial e.g. /dev/ttyUSB0 or COM1, TCP e.g. 127.0.0.1 )\n   -p Network port ( only required if type is TCP, default 1502 )\n   -b Baud rate ( only required if type is serial, default 9600 )\n   -d Directory for modbus data ( default is parent directory \"..\" )\n   -f File name for modbus data ( default is \"testdata.csv\" )\n   -v Verbose/Debug\n   -h Show this help page\n\n\nFormat for testdata (first row with colum title not to include):\n\nRegister;Datatype;Register Length;Is Dynamic;Value\n40001;1;1;0;123\n40012;2;2;0;654324\n40024;3;2;0;1234.7654\n40036;4;10;0;Hallo Neun Zeichen\n40100;3;2;1;../dynamic_data.csv:1\n\nDatatypes:\n1: int16\n2: int32\n3: float32  ( Decimal Seperator is \".\", see testdata example )\n4: string   ( Last byte is always 0x03, if string is longer than double the \"Register Length\" - 1, the the last byte of the last register is replaced with 0x03 )\"\n\nDynamic data:\n:1 after the filepath stands for :n'th colum of csv, starting at 0.\nEvery read cycle that starts in the specified Register reads one row further in dynamic_data.csv.\n\nIn your first read for Registers 40100 and 40101, you would get 98.76,\nin the second 87.65,\n...\nafter the last value is read from the dynamic_data.csv it starts over with the first one.\n\nexample for dynamic_data.csv\n123.456;98.76\n234.567;87.65\n345.678;76.54\n456.789;65.43\n"

class InputParser {
public:
    InputParser (int &argc, char **argv){
        for (int i=1; i < argc; ++i)
            this->tokens.emplace_back(argv[i]);
    }

    const std::string& getCmdOption(const std::string &option) const{
        std::vector<std::string>::const_iterator itr;
        itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
        if (itr != this->tokens.end() && ++itr != this->tokens.end()){
            return *itr;
        }
        static const std::string empty_string;

        return empty_string;
    }

    bool cmdOptionExists(const std::string &option) const{
        return std::find(this->tokens.begin(), this->tokens.end(), option)
               != this->tokens.end();
    }
private:
    std::vector <std::string> tokens;
};

std::string uint16ToBinary(uint16_t data) {
    std::string ret;
    //std::string ret = std::to_string(data) + ": ";
    for (int i = 15; i >= 0; --i) {
        if (i == 7) {
            ret += " ";
        }
        ret += ((data & (0x0001 << i)) >> i) ? "1": "0";
    }
    ret += " ";
    return ret;
}

void parseCsvToModbusData(mb::ModbusServer* modbusServer, const std::string& pathToFile, const std::string& fileName);

void generateInputDataStructs(mb::ModbusServer* modbusServer);
void generateFunctionDataStructs(mb::ModbusServer* modbusServer);

std::map<std::string, int> dynamic_file_counter;

std::string getLine(std::fstream& file, int num){
    auto line_count = (int) std::count(std::istreambuf_iterator<char>(file),
                                 std::istreambuf_iterator<char>(), '\n');

    int line_index = (num % line_count);

    std::string line;
    file.seekg(std::ios::beg);
    for(int i=0; i < line_index; ++i) {
        file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    }
    std::getline(file, line);

    return line;
}

inline std::vector<std::string> split(const std::string& str, char delim) {
    std::stringstream test(str);
    std::string segment;
    std::vector<std::string> seglist;

    while(std::getline(test, segment, delim))
    {
        seglist.push_back(segment);
    }

    return seglist;
}


std::vector<uint16_t> test_mb_function_pointer(mb::modbus_register_data *data, const mb::telegram& t) {

    auto arr = split(data->function_pointer.ptr_arg, ':');

    std::vector<uint16_t> ret(data->total_register_length);

    std::fstream file(arr[0]);
    if ( !file.is_open()) {
        std::cerr << "Error reading test file" << std::endl;
        std::cerr << "For Syntax info see argument -h" << std::endl;
        exit(1);
    }
    std::vector<uint16_t> tmp;
    std::string value = getLine(file, dynamic_file_counter[data->function_pointer.ptr_arg]++);
    if (arr.size() > 1) {
        value = split(value, ';')[std::stoi(arr[1])];
    }

    switch (data->function_pointer.type) {
        case mb::INT16:
            tmp.emplace_back( (uint16_t) std::stoi(value));
            break;
        case mb::INT32:
            tmp = mb::ModbusServer::int32ToRegisterData(std::stoi(value));
            break;
        case mb::FLOAT32:
            tmp = mb::ModbusServer::floatToRegisterData(std::stof(value));
            break;
        default:
            tmp = mb::ModbusServer::stringToRegisterData(value);
    }

    for (int i = 0; (i < data->total_register_length) && (i < tmp.size()); ++i) {
        ret[i] = tmp[i];
    }

    return ret;
}

int inputDataInt = 123;
float inputDataFloat = 2.34;
std::string inputDataString = "Test";

mb::ModbusServer *modbusServerPtr;

int serial_port;

void setup_serial(std::string serialPort) {

    serial_port = open(serialPort.c_str(), O_RDWR);

    // Check for errors
    if (serial_port < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
    }

}

int receive_serial(uint8_t* data_dest, int buffer_size) {
    int n = read(serial_port, &data_dest, buffer_size);

    return n;
}


bool send_serial(uint8_t* data_dest, int length) {
    return (-1 != write(serial_port, data_dest, sizeof(length)));
}


int main(int argc, char **argv){

    int n = 1;
    // little endian if true
    if(*(char *)&n == 1) {
        std::cout << "Little endian" << std::endl;
    } else {
        std::cout << "Big endian" << std::endl;
    }

    InputParser input(argc, argv);

    if (input.cmdOptionExists("-h")) {
        std::cout << HELP_TEXT << std::endl;
        exit(0);
    }

    mb::modbus_interface interface{.type=(mb::interface_type) DEFAULT_PROTOCOL, .config={.connection_point=DEFAULT_ADDR, .network_port=DEFAULT_PORT}, .debug=DEBUG};
    std::string option;
    std::string csvPath = DEFAULT_CSV_PATH;
    std::string csvFile = DEFAULT_CSV_FILE;

    try {
        if(input.cmdOptionExists("-i")){
            option = input.getCmdOption("-i");
            interface.type= (mb::interface_type) std::stoi(option);
        }
        if(input.cmdOptionExists("-a")){
            option = input.getCmdOption("-a");
            interface.config.connection_point = option;
        }
        if(input.cmdOptionExists("-p") && interface.type == mb::TCP){
            option = input.getCmdOption("-p");
            interface.config.network_port = std::stoi(option);
        }
        if(input.cmdOptionExists("-b") && interface.type != mb::TCP){
            option = input.getCmdOption("-b");
            interface.config.serial_baud = std::stoi(option);
        }
        if(input.cmdOptionExists("-d")){
            option = input.getCmdOption("-d");
            option = (option.c_str()[option.length()-1] == '/') ? option.substr(0, option.length() - 1) : option;
            csvPath = option;
        }
        if(input.cmdOptionExists("-f")){
            option = input.getCmdOption("-f");
            csvFile = option;
        }
        if(input.cmdOptionExists("-v")){
            interface.debug = true;
        }
    } catch (std::exception& e) {
        std::cerr << "TODO Argument Error" << std::endl;
        std::cerr << "For Syntax info see argument -h" << std::endl;
        exit(1);
    }
    mb::modbus_config config{
        .interface=interface,
        .ao_reg_count = 500,
        .ai_reg_count = 500
    };

    setup_serial(input.getCmdOption("-a"));

    modbusServerPtr = new mb::ModbusServer(config);

    modbusServerPtr->receiveFromInterface = &receive_serial;
    modbusServerPtr->sendToInterface = &send_serial;
    modbusServerPtr->setFd(serial_port);

    parseCsvToModbusData(modbusServerPtr, csvPath, csvFile);
    generateFunctionDataStructs(modbusServerPtr);
    generateInputDataStructs(modbusServerPtr);

    modbusServerPtr->printRegisterMapping();
    modbusServerPtr->setup();
    modbusServerPtr->connect();

    modbusServerPtr->setActiveNonBlocking();
    for (int i = 0; i < 1000; ++i) {
        modbusServerPtr->serveNonBlocking();
        std::this_thread::sleep_for(std::chrono::milliseconds (100));
    }

    modbusServerPtr->setInactiveNonBlocking();
    /*
    {
        modbusServerPtr->serve();
        modbusServerPtr->disconnect();
        std::cout << "schlafen 5 sec" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        modbusServerPtr->connect();
        std::cout << "wieder wach" << std::endl;
        modbusServerPtr->serve();
        modbusServerPtr->disconnect();
        std::cout << "schlafen 5 sec" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        modbusServerPtr->connect();
        std::cout << "wieder wach" << std::endl;
        modbusServerPtr->serve();
    }
    */
    modbusServerPtr->disconnect();
    modbusServerPtr->quit();

    /*
    std::cout << inputDataInt << std::endl;
    std::cout << inputDataFloat << std::endl;
    std::cout << inputDataString << std::endl;
    */

    delete modbusServerPtr;

    return 0;
}

void parseCsvToModbusData(mb::ModbusServer* modbusServer, const std::string& pathToFile, const std::string& fileName) {
    std::ifstream file(pathToFile + "/" + fileName);
    if ( !file.is_open()) {
        std::cerr << "Error reading test file" << std::endl;
        std::cerr << "For Syntax info see argument -h" << std::endl;
        exit(1);
    }
    std::string line;
    while (std::getline(file, line))
    {
        auto arr = split(line, ';');
        uint16_t reg = std::stoi(arr[0]);
        int datatype = std::stoi(arr[1]);
        uint8_t reg_len = std::stoi(arr[2]);
        std::string value = arr[4];
        bool dynamicData = ( "1" == arr[3] );

        //std::cout << "Register: " << reg << ", Datentyp: " << datatype  << ", Länge: " << reg_len <<", Wert: " << value << std::endl;

        std::vector<uint16_t> regData;
        std::vector<uint16_t> tmp;
        mb::modbus_register_data data {};

        if (dynamicData) {
            /// iterating file
            data = {
                    .type=mb::FUNCTION_POINTER,
                    .direction = mb::OUTPUT,
                    .address=reg,
                    .total_register_length=reg_len,

                    .function_pointer = {
                            .ptr = test_mb_function_pointer,
                            .ptr_arg = value
                    }
            };

            switch (datatype) {
                case 1:
                    /// int16
                    data.function_pointer.type = mb::INT16;
                    data.function_pointer.type_size = 1;
                    break;
                case 2:
                    /// int32
                    data.function_pointer.type = mb::INT32;
                    data.function_pointer.type_size = 2;
                    break;
                case 3:
                    /// float
                    data.function_pointer.type = mb::FLOAT32;
                    data.function_pointer.type_size = 2;
                    break;
                case 4:
                    /// string
                    data.function_pointer.type = mb::STRING;
                    break;
                default:
                    break;
            }
            dynamic_file_counter[value] = 0;
        } else {
            switch (datatype) {

                case 1:
                    /// int16
                    regData.emplace_back((uint16_t) std::stoi(value));
                    break;
                case 2:
                    /// int32
                    tmp = mb::ModbusServer::int32ToRegisterData((int)std::stoi(value));
                    for (int i = 0; i < reg_len; ++i) {
                        regData.emplace_back(tmp[i]);
                    }
                    break;
                case 3:
                    /// float
                    {
                        float f = (float)std::stof(value);
                        tmp = mb::ModbusServer::floatToRegisterData(f);
                        for (int i = 0; i < reg_len; ++i) {
                            regData.emplace_back(tmp[i]);
                        }
                    }break;
                case 4:
                    /// string
                    if (  mb::ModbusServer::getRegisterLengthForString(value) > reg_len) {
                        value = value.substr(0, (reg_len*2) - 1);
                    }
                    tmp = mb::ModbusServer::stringToRegisterData(value);
                    for (int i = 0; i < reg_len; ++i) {
                        regData.emplace_back(tmp[i]);
                    }
                    break;
                default:
                    break;
            }

            regData = mb::to_big_endian(regData);

            data = {
                    .type=mb::VALUE,
                    .register_type=mb::AO,
                    .direction=mb::OUTPUT,
                    .address=reg,
                    .value=regData
            };
        }

        modbusServer->setDataStructToRegister(data);
    }
}

void generateInputDataStructs(mb::ModbusServer* modbusServer) {
    struct mb::modbus_register_data inputDataStructInt;
    inputDataStructInt.type = mb::POINTER;
    inputDataStructInt.register_type = mb::AO;
    inputDataStructInt.direction = mb::BIDIRECTIONAL;
    inputDataStructInt.address = 40100;
    inputDataStructInt.data_pointer = {.type = mb::INT32, .ptr = &inputDataInt};

    struct mb::modbus_register_data inputDataStructFloat;
    inputDataStructFloat.type = mb::POINTER;
    inputDataStructFloat.register_type = mb::AO;
    inputDataStructFloat.direction = mb::BIDIRECTIONAL;
    inputDataStructFloat.address = 40104;
    inputDataStructFloat.data_pointer = {.type = mb::FLOAT32, .ptr = &inputDataFloat};

    struct mb::modbus_register_data inputDataStructString;
    inputDataStructString.type = mb::POINTER;
    inputDataStructString.register_type = mb::AO;
    inputDataStructString.direction = mb::BIDIRECTIONAL;
    inputDataStructString.address = 40108;
    inputDataStructString.max_string_length = 9;
    inputDataStructString.data_pointer = {.type = mb::STRING, .ptr = &inputDataString};

    modbusServer->setDataStructToRegister(inputDataStructInt);
    modbusServer->setDataStructToRegister(inputDataStructFloat);
    modbusServer->setDataStructToRegister(inputDataStructString);


    //modbusServer->removeDataStructFromRegister(inputDataStructInt);

}

std::vector<uint16_t> mb_exit(mb::modbus_register_data *data, const mb::telegram& t = {}) {
    modbusServerPtr->setInactive();
    return std::vector<uint16_t>{0x0000};
}

std::vector<uint16_t> mb_setInt16(mb::modbus_register_data *data) {
    data->value;
    return std::vector<uint16_t>{0x0000};
}

void generateFunctionDataStructs(mb::ModbusServer* modbusServer) {
    struct mb::modbus_register_data inputDataStructExit;
    inputDataStructExit.type = mb::FUNCTION_POINTER;
    inputDataStructExit.register_type = mb::AO;
    inputDataStructExit.direction = mb::OUTPUT;
    inputDataStructExit.address = 40420;
    inputDataStructExit.function_pointer = {
            .type = mb::INT16,
            .type_size = 1,
            .ptr = mb_exit,
    };

    modbusServer->setDataStructToRegister(inputDataStructExit);

    inputDataStructExit = {};
    inputDataStructExit.type = mb::FUNCTION_POINTER;
    inputDataStructExit.register_type = mb::AO;
    inputDataStructExit.direction = mb::BIDIRECTIONAL;
    inputDataStructExit.address = 40422;
    inputDataStructExit.function_pointer = {
            .type = mb::INT16,
            .type_size = 1,
            .ptr = mb_exit,
    };

    modbusServer->setDataStructToRegister(inputDataStructExit);
}