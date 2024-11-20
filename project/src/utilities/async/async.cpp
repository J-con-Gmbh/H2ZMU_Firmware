//
// Created by jriessner on 06.07.23.
//

#include <future>
#include <iostream>
#include <algorithm>

#include "epoch_time.h"
#include "utilities/async/async.h"
#include "Core.h"

std::map<int, async::async_handle> async::Handler::handle_dictionary;

namespace async {

    std::map<int, async_handle> get_handle_dictionary() {
        return {
                {e_default_handle                     , default_handle},
                {e_handle_psensor_update              , handle_psensor_update},
                {e_handle_tsensor_update              , handle_tsensor_update},
                {e_handle_test                        , handle_test},
                {e_print_sensordata_statistics        , print_sensordata_statistics},
                {e_update_sensordata_for_statistics   , update_sensordata_for_statistics},
                {e_handle_sensordata_stream           , handle_sensordata_stream},
                {e_handle_button_update               , handle_button_update},
                {e_handle_sensorstate_update          , handle_sensorstate_update},
                {e_handle_websocket_remotecontrol     , handle_websocket_remotecontrol},
                {e_handle_plc_interface               , handle_plc_interface},
                {e_handle_plc_firmware_update_via_uart, handle_plc_firmware_update_via_uart},
                {e_handle_gui                         , handle_gui},
                {e_handle_di                          , handle_di}
        };
    }

    std::map<int, struct async::async_job> get_handles() {
        Handler::handle_dictionary = get_handle_dictionary();

        // TODO aus YAML oder Datenbank auslesen
        std::map<int, struct async::async_job> jobs;

        struct async_job job = {};
        job.id = e_default_handle;
        job.active = true;
        job.name = "Test";
        job.freq_ms = 2000;
        job.exec = Handler::handle_dictionary[e_default_handle];
        //jobs.insert(std::make_pair(job.id, job));

        job = {};
        job.id = e_handle_psensor_update;
        job.active = true;
        job.name = "Pressure sensor data update";
        job.freq_ms = 1000;
        job.exec = Handler::handle_dictionary[e_handle_psensor_update];
        jobs.insert(std::make_pair(job.id, job));

        job = {};
        job.id = 2;
        job.active = true;
        job.name = "Temp sensor data update";
        job.freq_ms = 1000;
        job.exec = Handler::handle_dictionary[e_handle_tsensor_update];
        jobs.insert(std::make_pair(job.id, job));

        job = {};
        job.id = 3;
        job.active = true;
        job.name = "Test handle";
        job.freq_ms = 2000;
        job.exec = Handler::handle_dictionary[e_handle_test];
        //jobs.insert(std::make_pair(job.id, job));

        job = {};
        job.id = 4;
        job.active = false;
        job.name = "Print sensordata statistics";
        job.freq_ms = 10000;
        job.exec = Handler::handle_dictionary[e_print_sensordata_statistics];
        //jobs.insert(std::make_pair(job.id, job));

        job = {};
        job.id = 5;
        job.active = false;
        job.name = "Update sensordata for statistics";
        job.freq_ms = 1000;
        job.exec = Handler::handle_dictionary[e_update_sensordata_for_statistics];
        //jobs.insert(std::make_pair(job.id, job));

        job = {};
        job.id = 6;
        job.active = false;
        job.name = "Stream sensor data via uart";
        job.freq_ms = 1000;
        job.exec = Handler::handle_dictionary[e_handle_sensordata_stream];
        //jobs.insert(std::make_pair(job.id, job));

        job = {};
        job.id = 7;
        job.active = true;
        job.name = "Update button states";
        job.freq_ms = 100;
        job.exec = Handler::handle_dictionary[e_handle_button_update];
        jobs.insert(std::make_pair(job.id, job));

        job = {};
        job.id = 8;
        job.active = false;
        job.name = "Update sensor states";
        job.freq_ms = 2000;
        job.exec = Handler::handle_dictionary[e_handle_sensorstate_update];
        //jobs.insert(std::make_pair(job.id, job));

        job = {};
        job.id = 9;
        job.active = false; //Core::instance->config->isTrue("RC_ACTIVE");
        job.name = "Websocket remote control";
        job.freq_ms = 100;
        job.exec = Handler::handle_dictionary[e_handle_websocket_remotecontrol];
        //jobs.insert(std::make_pair(job.id, job));

        job = {};
        job.id = 10;
        job.active = true; //Core::instance->config->isTrue("RC_ACTIVE");
        job.name = "Handle Modbus connection non blocking";
        job.freq_ms = 10;
        job.exec = Handler::handle_dictionary[e_handle_plc_interface];
        //jobs.insert(std::make_pair(job.id, job));

        job = {};
        job.id = 11;
        job.active = false; //Core::instance->config->isTrue("RC_ACTIVE");
        job.name = "Receive file via UART on PLC Serial Port";
        job.freq_ms = (u_int) 10;
        job.exec = Handler::handle_dictionary[e_handle_plc_firmware_update_via_uart];
        job.loglvl = loglevel::intrest;
        jobs.insert(std::make_pair(job.id, job));

        job = {};
        job.id = 12;
        job.active = true;
        job.name = "Update Gui";
        job.freq_ms = 20000;
        job.exec = Handler::handle_dictionary[e_handle_gui];
        jobs.insert(std::make_pair(job.id, job));

        job = {};
        job.id = 13;
        job.active = true;
        job.name = "Handle Digital In";
        job.freq_ms = 1000;
        job.exec = Handler::handle_dictionary[e_handle_di];
        jobs.insert(std::make_pair(job.id, job));

        return jobs;
    }

    Handler::Handler(bool async) {
        this->async = async;
        this->registered_handles = get_handles();
    }

    bool Handler::aSync() {
        long long now = utils::epoch_time::getUnixTimestampMs();

        std::vector<int> toErase;
        for (const auto &result: this->results) {
            if (result.second.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                //std::cout << "Job Exec Name:  ready " << this->registered_handles[result.first].name << std::endl;
                toErase.emplace_back(result.first);
                struct async_job handle = this->registered_handles[result.first];
                long exec_time_ms = now - handle.last_run;
                if (!result.second.get()) {
                    Log::write({
                                       .message="Async Job [ " + handle.name + " ] Failed with " +
                                                std::to_string(exec_time_ms) + " ms execution time",
                                       .thrownAt=__PRETTY_FUNCTION__,
                                       .loglvl=handle.loglvl
                               });
                }
            }
        }
        for (const auto &item: toErase) {
            this->results.erase(item);
            this->registered_handles[item].running = false;
            //std::cout << "Job Exec Name:  erase " << this->registered_handles[item].name << std::endl;
        }

        std::lock_guard<std::mutex> lock(this->mtx_handles);
        //long long now = utils::epoch_time::getUnixTimestampMs();
        for (std::pair<int, async_job> handle: this->registered_handles) {
            if (!handle.second.active) {  
                continue;
            }
            long long diff = now - handle.second.last_run;
            if (diff >= handle.second.freq_ms) {
                if (
                        this->registered_handles[handle.first].running
                        && handle.second.freq_ms != (u_int) -1 /// if freq is -1, handle is a one time job
                ) {
                    //std::cout << "async job still running: " << handle.second.name << std::endl;
                    this->registered_handles[handle.first].last_run += (int)((float)handle.second.freq_ms * 0.3);
                    continue;
                }
                this->registered_handles[handle.first].running = true;
                std::shared_future<bool> future = std::async(std::launch::async, handle.second.exec);

                this->registered_handles[handle.first].last_run = now;

                results.insert(std::make_pair(handle.first, std::move(future)));

                if (handle.second.singlerun) {
                    this->setHandleActive(handle.first, false);
                }
            }
        }

        return true;
    }

    bool Handler::inSync() {
        bool ret = true;
        long long now = utils::epoch_time::getUnixTimestampMs();
        for (std::pair<int, async_job> handle: this->registered_handles) {
            if (!handle.second.active) {
                continue;       
            }
            long long diff = now - handle.second.last_run;
            if ( diff >= handle.second.freq_ms ) {
                ret = handle.second.exec();
                this->registered_handles[handle.first].last_run = now;
            }
        }
        return ret;
    }

    bool Handler::runTask( const async_job& handle, long long ts, std::vector<std::shared_future<bool>> &results) {

        bool result = true;
        long long diff = ts - handle.last_run;
        if ( diff >= handle.freq_ms ) {
            std::shared_future<bool> future = std::async(std::launch::deferred, handle.exec);
            this->registered_handles[handle.id].last_run = ts;
            results.push_back(future);
        }

        return result;
    }

    bool Handler::handle() {
        bool ret;
        if (this->async) {
            ret = this->aSync();
        } else {
            ret = this->inSync();
        }

        return ret;
    }

    bool Handler::getHandleActive(int id) {
        if (this->registered_handles.count(id)) {

            return this->registered_handles[id].active;
        }
        return false;
    }

    bool Handler::setHandleActive(int id, bool active) {
        std::lock_guard<std::mutex> lock(this->mtx_handles);

        if (!this->registered_handles.count(id)) {
            return false;
        }

        this->registered_handles[id].active = active;

        return true;
    }

}
