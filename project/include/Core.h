//
// Created by jriessner on 30.05.23.
//

#ifndef H2ZMU_V1_CORE_H
#define H2ZMU_V1_CORE_H

#include <memory>

#include "conf/Config.h"
#include "data/db/DatabaseService.h"
#include "Log.h"
#include "utilities/async/async.h"
#include "utilities/control/Control.h"
#include "process_logic/measurement/measurement.h"
#include "interface/modbus/PlcInterface.h"
#include "Uart.h"
#include "interface/hmi/Gui.h"

class Core {
private:
    bool running = true;
    bool stopped = false;
    bool block_meas = true;

    bool measurementRunning = false;
    struct measurement msrmnt;

    bool startMeasurement(const std::string& externalId = "");

    bool stopMeasurement();
public:
    // MOVE TO PRIVATE WHEN DONE
    std::shared_ptr<DatabaseService> databaseService;
    static std::shared_ptr<Core> instance;
    std::shared_ptr<utils::Config> config;
    std::shared_ptr<PlcInterface> plcInterface;
    std::shared_ptr<uart::Uart> plcSerial;
    std::shared_ptr<async::Handler> asyncHandler;

    explicit Core(const std::string& pathToConfig);
    bool setupRepositories();
    std::shared_ptr<DatabaseService> getDatabaseService();
    const struct measurement& getCurrentMeasurement();

    void setup();
    void run();
    bool setBlockMeas(bool block);
    bool getBlockMeas();

    bool isCaching();

    Control control;
    void handleControls();

    bool closeall();
    bool shutdown();
    void kill();

};


#endif //H2ZMU_V1_CORE_H
