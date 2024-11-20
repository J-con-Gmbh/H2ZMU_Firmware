//
// Created by jriessner on 30.05.23.
//

#include <iostream>
#include <thread>

#include "Core.h"
#include "data/db/Migration.h"
#include "sys/dump_stacktrace.h"
#include "data/db/repositories/SensorRepository.h"
#include "data/db/repositories/BottleRepository.h"
#include "data/db/repositories/CascadeRepository.h"
#include "data/db/repositories/CascadestateRepository.h"
#include "data/db/repositories/ErrorRepository.h"
#include "data/db/repositories/WarningRepository.h"
#include "data/db/repositories/HardwareProtocolRepository.h"
#include "data/db/repositories/MeasurementRepository.h"
#include "data/db/repositories/OccurredErrorRepository.h"
#include "data/db/repositories/OccurredWarningRepository.h"
#include "data/db/repositories/ParamRepository.h"
#include "data/db/repositories/Repository.h"
#include "data/db/repositories/SensorstateRepository.h"
#include "data/db/repositories/TranslationRepository.h"
#include "data/db/repositories/UserRepository.h"
#include "data/db/repositories/VesselstateRepository.h"
#include "data/db/repositories/ArchiveRepository.h"
#include "utilities/async/async.h"
#include "rgfc/RgfCorr.h"
#include "vessel/Vessel.h"

#include "process_logic/measurement/dynamic/DynamicMeasurement.h"
#include "interface/hmi/Button.h"
#include "modbus_interface.h"
#include "interface/modbus/PlcInterface.h"

#include "interface/hmi/Gui.h"

std::shared_ptr<Core> Core::instance;

Core::Core(const std::string& pathToConfig) {

    this->databaseService = std::make_shared<DatabaseService>();

    this->config = std::make_shared<utils::Config>();
    this->config->loadConfigFile(pathToConfig);

    std::cout << this->config->dumpConfig(": ") << std::endl;

    if ( ! this->databaseService->initDb(
            this->config->getValue(CONF_DB_FILE),
            this->isCaching()
    ) ) {
        /// TODO if exiting -> print error-code on screen
        std::cerr << "Datenbank initialisation fehlgeschalgen!" << std::endl;
        exit(1);
    }
    try{
        Log::setLogLevel((loglevel)std::stoi(this->config->getValue("LOG_LVL")));
    } catch (std::exception &e) {}

    Log::open(this->config->getValue(CONF_LOG_DIR));
    Log::write({
               .message="Initializing complete",
               .thrownAt=__PRETTY_FUNCTION__,
               .loglvl=loglevel::intrest
               });

}

void Core::setup() {
    this->setupRepositories();

    Vessel::instance = std::make_shared<Vessel>();
    Vessel::instance->setup();

    Gui::instance = std::make_shared<Gui>();
    Gui::instance->init();

    struct mb::modbus_config plcConf = {
            .interface = {
                    // TODO Parametriesieren/Konfigurieren
                    .type=mb::SERIAL,
                    .config = {
                            .connection_point = "/tmp/pts/h2_plc_srv",
                            .serial_baud = 19200
                    },
                    .master = true,
            },
            .do_reg_count = 1000,
            .di_reg_count = 1000,
            .ao_reg_count = 1000,
            .ai_reg_count = 1000,
    };

    this->plcInterface = std::make_shared<PlcInterface>(plcConf);
    this->plcInterface->setup();

    this->plcSerial = std::make_shared<uart::Uart>();
    this->plcSerial->setup({
       .connection_point=plcConf.interface.config.connection_point
    });

    this->asyncHandler = std::make_shared<async::Handler>();

}

bool Core::isCaching() {
    return this->config->isTrue(CONF_DB_CACHING);
}

bool Core::setupRepositories() {

    BottleRepository::instance = std::make_shared<BottleRepository>();
    BottleRepository::instance->setup(this->databaseService);

    CascadeRepository::instance = std::make_shared<CascadeRepository>();
    CascadeRepository::instance->setup(this->databaseService);

    CascadestateRepository::instance = std::make_shared<CascadestateRepository>();
    CascadestateRepository::instance->setup(this->databaseService);

    ErrorRepository::instance = std::make_shared<ErrorRepository>();
    ErrorRepository::instance->setup(this->databaseService);

    WarningRepository::instance = std::make_shared<WarningRepository>();
    WarningRepository::instance->setup(this->databaseService);

    HardwareProtocolRepository::instance = std::make_shared<HardwareProtocolRepository>();
    HardwareProtocolRepository::instance->setup(this->databaseService);

    MeasurementRepository::instance = std::make_shared<MeasurementRepository>();
    MeasurementRepository::instance->setup(this->databaseService);

    OccurredErrorRepository::instance = std::make_shared<OccurredErrorRepository>();
    OccurredErrorRepository::instance->setup(this->databaseService);

    OccurredWarningRepository::instance = std::make_shared<OccurredWarningRepository>();
    OccurredWarningRepository::instance->setup(this->databaseService);

    ParamRepository::instance = std::make_shared<ParamRepository>();
    ParamRepository::instance->setup(this->databaseService);

    SensorRepository::instance = std::make_shared<SensorRepository>();
    SensorRepository::instance->setup(this->databaseService);

    SensorstateRepository::instance = std::make_shared<SensorstateRepository>();
    SensorstateRepository::instance->setup(this->databaseService);

    TranslationRepository::instance = std::make_shared<TranslationRepository>();
    TranslationRepository::instance->setup(this->databaseService);

    UserRepository::instance = std::make_shared<UserRepository>();
    UserRepository::instance->setup(this->databaseService);

    VesselstateRepository::instance = std::make_shared<VesselstateRepository>();
    VesselstateRepository::instance->setup(this->databaseService);

    ArchiveRepository::instance = std::make_shared<ArchiveRepository>();
    ArchiveRepository::instance->setup(this->databaseService);

    return true;
}

enum multimsrmntstage {
    idle = 0,   // Pressure is stable
    p_decr = 1, // Pressure is decreasing -> H2 flow
    t_decr = 2, // Temperature decrease in one of the cascades has begun -> map cascade. Occurs parallel to multimsrmntstage::p_decr
    p_wfj = 3,  // Waiting for pressure to jump up -> cycle restarting
};

struct multimeasurementstate {
    bool running;
    struct measurement measurement;
    enum multimsrmntstage mmse = multimsrmntstage::idle;
    int currentCascade = -1;
    long currentPressureDecreaseStartTs = 0;
    dataset currentPressureDecreaseStartDataset;
    float tempStart = -1;
    float tempEnd = -1;
    float currentPressureDecreaseStartValue = -1;
    int currentCascadeId;
};

void Core::run() {

    while (this->running) {
        this->asyncHandler->handle();
        this->handleControls();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    this->stopped = true;
}

bool Core::setBlockMeas(bool block)
{
    this->block_meas = block;
    return this->block_meas;
}

bool Core::getBlockMeas()
{
    return this->block_meas;
}

std::shared_ptr<DatabaseService> Core::getDatabaseService() {
    return this->databaseService;
}

void Core::handleControls() {

    std::tuple<bool, struct Control::command> ret;

    while (std::get<bool>(ret = this->control.receive())) {

        struct Control::command cmd = std::get<struct Control::command>(ret);
        switch (cmd.action) {

            case core::control::action::MEASUREMENT_START:
                {
                    cmd.returnValue = this->startMeasurement(cmd.input);
                    if (cmd.returnValue) {
                        this->control.lock(cmd.interface);
                    }
                    cmd.output = std::to_string(this->msrmnt.id);
                    std::string msg = (cmd.returnValue) ? "Messung erfolgreich gestartet (Nr " + std::to_string(this->msrmnt.id) + ")": "Messung starten Fehlgeschlagen!";
                    Log::write({
                        .message=msg,
                        .interface=interface::remote_control,
                        .thrownAt=__PRETTY_FUNCTION__,
                        .loglvl=loglevel::intrest
                    },
                    (cmd.returnValue) ? logfile::info : logfile::error
                    );
                    PlcInterface::measurement_start = false;

                } break;

            case core::control::action::MEASUREMENT_STOP:
                {
                    cmd.returnValue = this->stopMeasurement();
                    std::string msg = (cmd.returnValue) ? "Messung erfolgreich beendet (Nr " + std::to_string(this->msrmnt.id) + ")" : "Messung beenden Fehlgeschlagen!";
                    Log::write({
                                       .message=msg,
                                       .interface=interface::remote_control,
                                       .thrownAt=__PRETTY_FUNCTION__,
                                       .loglvl=loglevel::intrest
                               },
                               (cmd.returnValue) ? logfile::info : logfile::error
                    );

                    PlcInterface::measurement_stop = false;
                    Gui::instance->pushMeasurement(this->msrmnt.id);
                } break;
        }
        cmd.finished = true;
        this->control.update(cmd);
    }
}

bool Core::startMeasurement(const std::string& externalId) {
    auto result = msrmnt::create_measurement(0, externalId);
    if ( !std::get<bool>(result)) {
        return false;
    }
    bool ret = std::get<bool>(result);
    if (ret) {
        this->msrmnt = std::get<measurement>(result);
    } else {
        return false;
    }
    ret = msrmnt::start_measurement(this->msrmnt);

    return ret;
}
bool Core::stopMeasurement() {
    bool ret = msrmnt::end_measurement(this->msrmnt);

#ifndef NDEBUG
    //std::cout << msrmnt::dump_measurement(this->msrmnt) << std::endl;
#endif

    return ret;
}

const struct measurement& Core::getCurrentMeasurement() {
    return this->msrmnt;
}

bool Core::closeall() {
    // TODO
    return true;
}

bool Core::shutdown() {
    if (this->measurementRunning) {
        return false;
    }
    this->running = false;
    return true;
}

void Core::kill() {
    this->running = false;
    while (!this->stopped) {}
    std::cout << "Core::kill()" << std::endl;
}
