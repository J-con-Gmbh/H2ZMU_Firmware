#include <gtest/gtest.h>
#include <thread>
#include "data/db/DatabaseService.h"
#include "data/db/repositories/ParamRepository.h"
#include "data/db/repositories/OccurredErrorRepository.h"
#include "data/db/repositories/ErrorRepository.h"
#include "data/db/repositories/OccurredWarningRepository.h"
#include "data/db/repositories/WarningRepository.h"
#include "data/db/repositories/VesselstateRepository.h"
#include "data/db/repositories/SensorstateRepository.h"
#include "data/db/repositories/CascadestateRepository.h"
#include "data/db/repositories/CascadeRepository.h"
#include "data/db/repositories/BottleRepository.h"
#include "conf/Config.h"
#include "data/db/Migration.h"
#include "sys/dump_stacktrace.h"
#include "Core.h"
#include "utilities/modbus/ModbusDataConversion.h"
#include "process_logic/initialsetup/InitialSetupDataCache.h"

std::shared_ptr<utils::Config> config;

TEST(BuildTests, BuildTest)
{
    ASSERT_EQ(1, 1);
}

TEST(RepositoryTests, ErrorTest)
{
    ASSERT_EQ(true, ErrorRepository::instance->loadAll());
    ASSERT_EQ(5, ErrorRepository::instance->getAllErrors().size());
}

TEST(RepositoryTests, OccurredErrorTest)
{
    for (int i = 0; i < 1100; i++)
    {
        ASSERT_EQ(true, OccurredErrorRepository::instance->logError({
                                                    .errCode = "E001",
                                                    .interface = hardwareInterface::runtime,
                                                    .location = __PRETTY_FUNCTION__,
                                                    .data = utils::sys::dump_stacktrace(),
                                                    }));
    }

    /// Test in memory representation
    ASSERT_EQ(100, OccurredErrorRepository::instance->getAllOccurredErrors().begin()->first);
    ASSERT_EQ(1099, OccurredErrorRepository::instance->getAllOccurredErrors().rbegin()->first);
    ASSERT_EQ(1000, OccurredErrorRepository::instance->getAllOccurredErrors().size());

    /// Test database representation
    ASSERT_EQ(true, OccurredErrorRepository::instance->loadAll());

    ASSERT_EQ(100, OccurredErrorRepository::instance->getAllOccurredErrors().begin()->first);
    ASSERT_EQ(1099, OccurredErrorRepository::instance->getAllOccurredErrors().rbegin()->first);
    ASSERT_EQ(1000, OccurredErrorRepository::instance->getAllOccurredErrors().size());

    struct occurrederror _error = OccurredErrorRepository::instance->getOccurredErrorById(0); 
    //ASSERT_EQ(true, _error.resolved);
}

TEST(RepositoryTests, WarningTest)
{
    ASSERT_EQ(true, WarningRepository::instance->loadAll());
    ASSERT_EQ(3, WarningRepository::instance->getAllWarnings().size());
}

TEST(RepositoryTests, OccurredWarningTest)
{
    for (int i = 0; i < 10; i++)
    {
        ASSERT_EQ(true, OccurredWarningRepository::instance->logWarning({
                                                    .warnCode = "W001",
                                                    .interface = hardwareInterface::runtime,
                                                    .location = __PRETTY_FUNCTION__,
                                                    .data = utils::sys::dump_stacktrace(),
                                                    }));
    }
    ASSERT_EQ(true, OccurredWarningRepository::instance->loadAll());
    ASSERT_EQ(10, OccurredWarningRepository::instance->getAllOccurredWarnings().size());
}

TEST(RepositoryTests, CascadeRepositoryTest)
{
    struct cascade _cascade;
    _cascade.fk_pressure_sensor_upper = 1;
    CascadeRepository::instance->createCascade(_cascade);
    ASSERT_EQ(true, CascadeRepository::instance->loadAll());
    ASSERT_EQ(0, _cascade.id);
}

TEST(RepositoryTests, BottleRepositoryTest)
{
    struct cascade _cascade = CascadeRepository::instance->getCascadeById(0);
    struct bottle _bottle;
    _bottle.fk_cascade = _cascade.id;
    _bottle.fk_sensor = 1;
    BottleRepository::instance->createBottle(_bottle);
    ASSERT_EQ(true, BottleRepository::instance->loadAll());
    // 5 Because there are already four entries in database
    ASSERT_EQ(0, _cascade.id);
    ASSERT_EQ(0, _bottle.id);
}

TEST(RepositoryTests, VesselStateTest)
{
    for (int i = 0; i < 1100; i++)
    {
        ASSERT_EQ(true, VesselstateRepository::instance->newVesselState(VesselstateRepository::instance->createVesselState()));
    }

    ASSERT_EQ(true, VesselstateRepository::instance->loadAll());
    ASSERT_EQ(1000, VesselstateRepository::instance->getAllVesselstates().size());
}

TEST(RepositoryTests, SensorStateTest)
{
    for (int i = 0; i < 1100; i++)
    {
        struct sensorstate sstate = {.id = i};
        ASSERT_EQ(true, SensorstateRepository::instance->persistSensorstate(sstate));
    }

    ASSERT_EQ(true, SensorstateRepository::instance->loadAll());
    ASSERT_EQ(1000, SensorstateRepository::instance->getAllSensorstates().size());
}

TEST(RepositoryTests, CascadeStateTest)
{
    for (int i = 0; i < 1100; i++)
    {
        struct cascadestate cstate = {.id = i};
        cstate.norm_volume = 0;
        ASSERT_EQ(true, CascadestateRepository::instance->newCascadestate(cstate));
    }

    ASSERT_EQ(true, CascadestateRepository::instance->loadAll());
    ASSERT_EQ(1000, CascadestateRepository::instance->getAllCascadestates().size());
}

TEST(ModbusDataDeSerialisation, SensorDeSerialisation)
{

    std::vector<uint16_t> data;
    std::vector<uint8_t> data_bytes;

    auto sysToBigEndian = [] (
            const std::vector<u_int16_t>& vector
    ) -> std::vector<u_int16_t> {
        std::vector<u_int16_t> ret = mb::from_to_endian(vector, mb::determine_endianness(), {true, true});

        return ret;
    };

    std::vector<uint16_t> tmp = sysToBigEndian(simpleTypeToRegister<int>(2));
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = sysToBigEndian(simpleTypeToRegister<int>(3));
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = sysToBigEndian(simpleTypeToRegister<int>(4));
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = stringToRegisterData("asd asd asd asd asd");
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = stringToRegisterData("bsd asd asd asd asd");
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = stringToRegisterData("csd asd asd asd asd");
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = sysToBigEndian(simpleTypeToRegister<float>(5.5));
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = sysToBigEndian(simpleTypeToRegister<float>(6.6));
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = sysToBigEndian(simpleTypeToRegister<int>(7));
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = sysToBigEndian(simpleTypeToRegister<int>(8));
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = sysToBigEndian(simpleTypeToRegister<float>(9.9));
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
    PlcInterface::mb_receiveVesselData_Sensor({}, {.data=data_bytes, .data_len=static_cast<uint8_t>(len)});

    struct sensor snsr = InitialSetupDataCache::get_setupdata().sensors.back();

    ASSERT_EQ(snsr.id, 2);
    ASSERT_EQ(snsr.type, 3);
    ASSERT_EQ(snsr.type_order, 4);
    ASSERT_EQ(snsr.serialnumber, "asd asd asd asd asd");
    ASSERT_EQ(snsr.name, "bsd asd asd asd asd");
    ASSERT_EQ(snsr.manufacturer, "csd asd asd asd asd");
    EXPECT_NEAR(snsr.uppermeasurelimit_manufacturer, 5.5, 0.0001);
    EXPECT_NEAR(snsr.lowermeasurelimit_manufacturer, 6.6, 0.0001);
    ASSERT_EQ(snsr.fk_hardwareprotocol, 7);
    ASSERT_EQ(snsr.hardwareprotocol_address, 8);
    EXPECT_NEAR(snsr.offset, 9.9, 0.0001);

}

TEST(ModbusDataDeSerialisation, CascadeDeSerialisation)
{

    std::vector<uint16_t> data;
    std::vector<uint8_t> data_bytes;

    auto sysToBigEndian = [] (
            const std::vector<u_int16_t>& vector
    ) -> std::vector<u_int16_t> {
        std::vector<u_int16_t> ret = mb::from_to_endian(vector, mb::determine_endianness(), {true, true});

        return ret;
    };

    std::vector<uint16_t> tmp = sysToBigEndian(simpleTypeToRegister<int>(2));
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = sysToBigEndian(simpleTypeToRegister<int>(3));
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = sysToBigEndian(simpleTypeToRegister<int>(4));
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

    struct cascade cscd = InitialSetupDataCache::get_setupdata().cascades.back();

    ASSERT_EQ(cscd.id, 2);
    ASSERT_EQ(cscd.fk_pressure_sensor_upper, 3);
    ASSERT_EQ(cscd.fk_pressure_sensor_lower, 4);
}

TEST(ModbusDataDeSerialisation, BottleDeSerialisation)
{

    std::vector<uint16_t> data;
    std::vector<uint8_t> data_bytes;

    auto sysToBigEndian = [] (
            const std::vector<u_int16_t>& vector
    ) -> std::vector<u_int16_t> {
        std::vector<u_int16_t> ret = mb::from_to_endian(vector, mb::determine_endianness(), {true, true});

        return ret;
    };

    std::vector<uint16_t> tmp = sysToBigEndian(simpleTypeToRegister<int>(2));
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = stringToRegisterData("asd asd asd asd asd");
    data.insert(data.end(), tmp.begin(), tmp.end());


    tmp = sysToBigEndian(simpleTypeToRegister<int>(3));
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = sysToBigEndian(simpleTypeToRegister<int>(4));
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = sysToBigEndian(simpleTypeToRegister<int>(5));
    data.insert(data.end(), tmp.begin(), tmp.end());


    tmp = sysToBigEndian(simpleTypeToRegister<float>(5.5));
    data.insert(data.end(), tmp.begin(), tmp.end());

    tmp = stringToRegisterData("bsd asd asd asd asd");
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
    auto len = data_bytes.size();
    PlcInterface::mb_receiveVesselData_Bottle({}, {.data=data_bytes, .data_len=static_cast<uint8_t>(len)});

    struct bottle btl = InitialSetupDataCache::get_setupdata().bottles.back();

    ASSERT_EQ(btl.id, 2);
    ASSERT_EQ(btl.serialnumber, "asd asd asd asd asd");
    ASSERT_EQ(btl.fk_cascade, 3);
    ASSERT_EQ(btl.cascade_order, 4);
    ASSERT_EQ(btl.fk_sensor, 5);
    EXPECT_NEAR(btl.tara, 5.5, 0.0001);
    ASSERT_EQ(btl.manufacturer, "bsd asd asd asd asd");
    ASSERT_EQ(btl.builtyear, 2020);
    ASSERT_EQ(btl.nextcheck, 2035);
    EXPECT_NEAR(btl.vol_0, 245.4, 0.0001);
    EXPECT_NEAR(btl.pressure_0, 1.0, 0.0001);
    EXPECT_NEAR(btl.vol_ref, 255.8, 0.0001);
    EXPECT_NEAR(btl.pressure_ref, 489.0, 0.0001);

}


int main(int argc, char** argv)
{
    for (int i = 0; i < argc; ++i) {
        std::cout << "Arg(" << i << "):" << argv[i] << std::endl;
    }

    std::string confFilePath = std::string(argv[argc - 1]);

    if (confFilePath.substr(0,1) != "/") {
        return 0;
    }

    Core::instance = std::make_shared<Core>(confFilePath + "/config.txt");

    //Migration::removeDbFile(Core::instance->config->getValue("DB_FILE"));
    //std::this_thread::sleep_for(std::chrono::seconds(1));

    Migration::init(Core::instance->config, Core::instance->getDatabaseService());
    Migration::resetMigrations();
    Migration::migrate();
    Core::instance->setup();

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
