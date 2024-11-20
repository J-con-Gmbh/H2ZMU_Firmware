//
// Created by jriessner on 31.05.23.
//

#include <iostream>
#include <thread>
#include <cmath>
#include <csignal>

#include "Core.h"

#ifndef NDEBUG
#include <bitset>

#include "data/db/Migration.h"
#include "rgfc/RgfCorr.h"
#include "interface/hart/HartInterface.h"
#include "utilities/scale/Scale.h"
#include "interface/modbus/PlcInterface.h"
#include "modbus_interface.h"
#include "utilities/modbus/ModbusDataConversion.h"
#include "process_logic/initialsetup/InitialSetupDataCache.h"

void testsuite(int argc, char *argv[]) {

    if (argc <= 1) {
        std::cerr << "Pfad zur Konfiguration angeben!" << std::endl;
        exit(1);
    }

    std::string confFilePath = utils::strings::standardiseDirectoryPath(std::string(argv[1])) + "config.txt";

    Core::instance = std::make_shared<Core>(confFilePath);

    Migration::init(Core::instance->config, Core::instance->getDatabaseService());

    Migration::migrate();

    if (Core::instance->config->isTrue("MB_CLIENT_ACTIVE")) {
        auto modbusClient = std::make_shared<mb::ModbusClient>();
        modbusClient->setup(Core::instance->config->getValue("MB_CLIENT_FD"), false);
        HartInterface::init(modbusClient);
    }

    Core::instance->setup();

/*
    {

        std::vector<uint16_t> data;
        std::vector<uint8_t> data_bytes;

        auto sysToBigEndian = [] (
                const std::vector<u_int16_t>& vector
        ) -> std::vector<u_int16_t> {
            std::vector<u_int16_t> ret = mb::from_to_endian(vector, mb::determine_endianness(), {true, true});

            return ret;
        };

        auto stringToFixedLen = [] (
                std::string str,
                int byteLen = 19
        ) -> std::string {
            int diff = byteLen - str.length();
            if (diff > 0) {
                for (int i = 0; i < diff; ++i) {
                    str += " ";
                }
            } else {
                str = str.substr(0, byteLen);
            }

            return str;
        };


        std::vector<uint16_t> tmp = sysToBigEndian(simpleTypeToRegister<int>(1));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<int>(1));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<int>(-1));
        data.insert(data.end(), tmp.begin(), tmp.end());

        for (const auto &item: data) {
            union {
                uint16_t word;
                uint8_t bytes[2];
            };
            word = item;
            data_bytes.emplace_back(bytes[0]);
            data_bytes.emplace_back(bytes[1]);
        }
        auto len = data_bytes.size();
        PlcInterface::mb_receiveVesselData_Cascade({}, {.data=data_bytes, .data_len=static_cast<uint8_t>(len)});
        data = {};
        data_bytes = {};

        tmp = sysToBigEndian(simpleTypeToRegister<int>(1));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = stringToRegisterData(stringToFixedLen("TestBottle1", 19));
        data.insert(data.end(), tmp.begin(), tmp.end());


        tmp = sysToBigEndian(simpleTypeToRegister<int>(1));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<int>(1));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<int>(2));
        data.insert(data.end(), tmp.begin(), tmp.end());


        tmp = sysToBigEndian(simpleTypeToRegister<float>(90.1));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = stringToRegisterData(stringToFixedLen("RiGa Test"));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<int>(2020));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<int>(2035));
        data.insert(data.end(), tmp.begin(), tmp.end());


        tmp = sysToBigEndian(simpleTypeToRegister<float>(245.4));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<float>(1.0));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<float>(255.8));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<float>(489.0));
        data.insert(data.end(), tmp.begin(), tmp.end());


        for (const auto &item: data) {
            union {
                uint16_t word;
                uint8_t bytes[2];
            };
            word = item;
            data_bytes.emplace_back(bytes[0]);
            data_bytes.emplace_back(bytes[1]);
        }
        len = data_bytes.size();
        PlcInterface::mb_receiveVesselData_Bottle({}, {.data=data_bytes, .data_len=static_cast<uint8_t>(len)});
        data = {};
        data_bytes = {};


        tmp = sysToBigEndian(simpleTypeToRegister<int>(1));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<int>(2));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<int>(1));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = stringToRegisterData(stringToFixedLen("TestPressureSensor1"));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = stringToRegisterData(stringToFixedLen("TestPressureSensorN"));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = stringToRegisterData(stringToFixedLen("TestPressureSensorM"));
        data.insert(data.end(), tmp.begin(), tmp.end());


        tmp = sysToBigEndian(simpleTypeToRegister<float>(450));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<float>(1));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<int>(1));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<int>(1));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<float>(0.0));
        data.insert(data.end(), tmp.begin(), tmp.end());

        for (const auto &item: data) {
            union {
                uint16_t word;
                uint8_t bytes[2];
            };
            word = item;
            data_bytes.emplace_back(bytes[0]);
            data_bytes.emplace_back(bytes[1]);
        }
        len = data_bytes.size();
        PlcInterface::mb_receiveVesselData_Sensor({}, {.data=data_bytes, .data_len=static_cast<uint8_t>(len)});

        data = {};
        data_bytes = {};


        tmp = sysToBigEndian(simpleTypeToRegister<int>(2));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<int>(1));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<int>(1));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = stringToRegisterData(stringToFixedLen("TestTempSensor1"));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = stringToRegisterData(stringToFixedLen("TestTempSensorN"));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = stringToRegisterData(stringToFixedLen("TestTempSensorM"));
        data.insert(data.end(), tmp.begin(), tmp.end());


        tmp = sysToBigEndian(simpleTypeToRegister<float>(180));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<float>(-50));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<int>(1));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<int>(2));
        data.insert(data.end(), tmp.begin(), tmp.end());

        tmp = sysToBigEndian(simpleTypeToRegister<float>(0.0));
        data.insert(data.end(), tmp.begin(), tmp.end());

        for (const auto &item: data) {
            union {
                uint16_t word;
                uint8_t bytes[2];
            };
            word = item;
            data_bytes.emplace_back(bytes[0]);
            data_bytes.emplace_back(bytes[1]);
        }
        len = data_bytes.size();
        PlcInterface::mb_receiveVesselData_Sensor({}, {.data=data_bytes, .data_len=static_cast<uint8_t>(len)});


        bool ret = InitialSetupDataCache::validate();

        std::cout << ret << std::endl;
    }
*/
    // TODO Parametriesieren
    RgfCorr::setGlobalSharedPtr(medium::H2);

    Core::instance->plcInterface->connect();
    Core::instance->plcInterface->setActiveNonBlocking();

    Core::instance->run();

    Core::instance->plcInterface->setInactiveNonBlocking();

    std::cout << "Fertig" << std::endl;

}

#endif

void signalHandler( int signum ) {
    std::cout << "Interrupt signal (" << signum << ") received." << std::endl;
    // cleanup and close up stuff here
    // terminate program

    Core::instance->closeall();

    exit(0);
}

void show_mem_rep(char *start, int n)
{
    int i;
    for (i = 0; i < n; i++)
        printf(" %.2x", start[i]);
    printf("\n");
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

#ifndef NDEBUG
    testsuite(argc, argv);
#endif

    return 0;
}
