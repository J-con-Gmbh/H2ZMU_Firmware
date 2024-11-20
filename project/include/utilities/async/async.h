//
// Created by jriessner on 06.07.23.
//

#ifndef H2ZMU_V1_ASYNC_H
#define H2ZMU_V1_ASYNC_H

#include <string>
#include <vector>
#include <future>

#include "handles.h"
#include "Log.h"

using namespace h2zmu;

namespace async {

    typedef bool (*async_handle)();

    enum available_jobs {
        e_default_handle                      = 0,
        e_handle_psensor_update               = 1,
        e_handle_tsensor_update               = 2,
        e_handle_test                         = 3,
        e_print_sensordata_statistics         = 4,
        e_update_sensordata_for_statistics    = 5,
        e_handle_sensordata_stream            = 6,
        e_handle_button_update                = 7,
        e_handle_sensorstate_update           = 8,
        e_handle_websocket_remotecontrol      = 9,
        e_handle_plc_interface                = 10,
        e_handle_plc_firmware_update_via_uart = 11,
        e_handle_gui                          = 12,
        e_handle_gui_main_screen_update       = 13,
        e_handle_di                           = 14,

    };

    struct async_job {
        int id = -1;
        bool active = false;
        bool running = false;
        std::string name = "default";
        /// Delay between executions <br>When equals (u_int) -1, the job is deactivated after execution
        u_int freq_ms = 1000;
        long long last_run = 0;
        async_handle exec = default_handle;
        loglevel loglvl = loglevel::debug;
        bool singlerun = false;
    };

    std::map<int, async_handle> get_handle_dictionary();
    std::map<int, struct async::async_job> get_handles();

    class Handler {
    private:
        std::mutex mtx_handles;

        std::map<int, struct async::async_job> registered_handles;
        std::map<int, std::shared_future<bool>> results;
        bool async = true;
        bool running = false;
        bool runTask( const async_job& handle, long long ts, std::vector<std::shared_future<bool>> &results);

        bool aSync();
        bool inSync();
    public:
        explicit Handler(bool async = true);
        bool handle();
        bool getHandleActive(int id);
        bool setHandleActive(int id, bool active);

        static std::map<int, async_handle> handle_dictionary;
    };

}

#endif //H2ZMU_V1_ASYNC_H
