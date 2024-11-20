#include <cstdlib>
#include <iostream>
#include "Core.h"
#include "data/db/Migration.h"
#include "rgfc/RgfCorr.h"
#include "process_logic/initialsetup/InitialSetupDataCache.h"
#include "interface/hmi/Display.h"

int main(int argc, char *argv[]) {

    std::string path;
    if (argc > 1) {
        path = argv[1];
        path = path + "/";
    }

    Log::set_log_config({.entry_max_lines = 10, .print_log_to_stdout=3});
    Core::instance = std::make_shared<Core>(path + "config.txt");

    Migration::init(Core::instance->config, Core::instance->getDatabaseService());
    Migration::migrate();

    queue::Queues::instance = std::make_shared<queue::Queues>();

    // TODO Parametriesieren
    RgfCorr::setGlobalSharedPtr(medium::H2);

    Core::instance->setup();

    Display::instance = std::make_shared<Display>();
    Display::instance->initDisplay();

    Gui::instance = std::make_shared<Gui>();
    Gui::instance->init();

    Core::instance->plcInterface->connect();
    Core::instance->plcInterface->setActiveNonBlocking();

    Button::instance = std::make_shared<Button>();
    Button::instance->setup();
    // path: project/files/db/h2zmu.db
    // std::cout << "=========================================" << std::endl;
    // // ARCHIVE is a struct containing only a parameter struct for now
    // //   ParamRepository::updateParam
    // std::cout << Core::instance->databaseService->executeSqlReturn("INSERT INTO 'archive' ('nr', 'shortdescr', 'fk_description', 'datatype', 'unit', 'rolerestriction', 'switchrestriction', 'fk_errormsg', 'fk_hardwareinterface') VALUES (101, 'Test description', 1, 2, 'm', NULL, 1, 'err001', 3)") << std::endl;
    // std::string value = Core::instance->databaseService->executeSqlReturn("SELECT * FROM archive");
    // std::cout << value << std::endl;
    // std::cout << "=========================================" << std::endl;
    exit(0);
    Core::instance->run();

    Core::instance->plcInterface->setInactiveNonBlocking();

    std::cout << "Fertig" << std::endl;

    return EXIT_SUCCESS;
}