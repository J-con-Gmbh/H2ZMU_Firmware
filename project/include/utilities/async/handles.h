//
// Created by jriessner on 06.07.23.
//

#ifndef H2ZMU_V1_HANDLES_H
#define H2ZMU_V1_HANDLES_H

#include <map>
#include <string>

namespace async {

    class Cache {
    public:
        static std::map<std::string, std::string> handle_cache;
    };
    typedef bool (*async_handle)();

    bool default_handle();

    bool handle_tsensor_update();
    bool handle_psensor_update();
    bool handle_sensorstate_update();

    bool handle_test();
    bool print_sensordata_statistics();
    bool update_sensordata_for_statistics();
    bool handle_sensordata_stream();
    bool handle_button_update();
    bool handle_gui();
    bool handle_di();
    bool handle_websocket_remotecontrol();
    bool handle_plc_interface();
    bool handle_plc_firmware_update_via_uart();

};


#endif //H2ZMU_V1_HANDLES_H
