//
// Created by jriessner on 06.07.23.
//


#include <thread>

#include "utilities/async/handles.h"

#include "Core.h"
#include "vessel/Vessel.h"
#include "data/db/repositories/SensorRepository.h"
#include "Log.h"
#include "sys/FileHandler.h"
#include "sys/dump_stacktrace.h"
#include "utilities/queue/Queues.h"
#include "process_logic/sensordata/SensorTrendAnalyzer.h"
#include "interface/hmi/Button.h"
#include "interface/modules/dio16/AXL_SE_D16.h"
#include "data/db/repositories/OccurredErrorRepository.h"
#include "data/db/repositories/OccurredWarningRepository.h"

#include "interface/hmi/Gui.h"

//AXL_SE_D16 digitalIn1("Arp.Plc.Eclr/wDI16");

namespace async {
    std::map<std::string, std::string> Cache::handle_cache;

    bool default_handle () {
        std::cout << "default handle geschafft" << std::endl;
        Log::write({
                           .message="Async Handle not initialized!\n" + utils::sys::dump_stacktrace(),
                           .thrownAt=__PRETTY_FUNCTION__,
                           .loglvl=loglevel::warn
                   }
                   );

        std::cout << "default handle geschafft" << std::endl;

        return true;
    }


    bool handle_psensor_update () {
        
        std::vector<std::shared_ptr<PressureSensor>> pSensors = Vessel::instance->getPressureSensorsPtr();
        for (const auto &sensor: pSensors) {
            if (sensor) {
                float val = sensor->measure();
                auto epoch = utils::epoch_time::getUnixTimestamp();
                queue::Queues::instance->pressureSensorData.add(
                {
                    .sensor_id = sensor->getId(),
                    .timestamp = epoch,
                    .value = val
                }
                );
            }
        }

        Log::write({
                           .message="handle_psensor_update",
                           .thrownAt=__PRETTY_FUNCTION__,
                           .loglvl=loglevel::debug
                   });

        return true;
    }

    bool handle_tsensor_update () {
        std::vector<std::shared_ptr<TempSensor>> tSensors = Vessel::instance->getTempSensorsPtr();
        std::vector<std::tuple<int,long,float>> sensordata;
        for (const auto &sensor: tSensors) {
            if (sensor) {
                float val = sensor->measure();
                auto epoch = utils::epoch_time::getUnixTimestamp();
                queue::Queues::instance->temperatureSensorData.add(
                {
                    .sensor_id=sensor->getId(),
                    .timestamp=epoch,
                    .value=val
                }
                );
                std::fstream data;
                sensordata.emplace_back(std::make_tuple(sensor->getId(), utils::epoch_time::getUnixTimestamp(), val));
            }

        }
        for (const auto &item: sensordata) {
            queue::Queues::instance->temperatureSensorData.add(
            {
                .sensor_id=std::get<0>(item),
                .timestamp=std::get<1>(item),
                .value=std::get<2>(item)
            }
            );
        }


        Log::write({
                           .message="handle_tsensor_update",
                           .thrownAt=__PRETTY_FUNCTION__,
                           .loglvl=loglevel::debug
                   });
        return true;
    }

    bool handle_sensorstate_update() {
        std::cout << "handle_sensorstate_update" << std::endl;
        std::vector<std::shared_ptr<TempSensor>> tSensors = Vessel::instance->getTempSensorsPtr();
        std::vector<std::shared_ptr<PressureSensor>> pSensors = Vessel::instance->getPressureSensorsPtr();

        for (const auto &item: pSensors) {
            queue::Queues::instance->pressureSensorState.add(item->getSensorState());
        }
        for (const auto &item: tSensors) {
            queue::Queues::instance->temperatureSensorState.add(item->getSensorState());
        }

        return true;
    }

    bool handle_test() {
        std::cout << "Test Handle" << std::endl;

        return true;
    }

    bool print_sensordata_statistics() {
        std::cout << "print_sensordata_statistics" << std::endl;
        //SensorTrendAnalyzer::instance->updateAll();
        SensorTrendAnalyzer::instance->report();

        return true;
    }

    bool update_sensordata_for_statistics() {
        std::cout << "update_sensordata_for_statistics" << std::endl;
        SensorTrendAnalyzer::instance->updateAll();
        //SensorTrendAnalyzer::instance->report();  

        return true;
    }

    bool handle_sensordata_stream() {
        return true;
    }

    bool handle_button_update() {
        Button::instance->updateStatus();
        if(Button::instance->isAnyButtonPressed())
        {
            Gui::instance->update();
        }

        return true;
    }

    bool handle_gui() {
        Gui::instance->update();
        return true;
    }

    bool handle_di() {
        /*
        std::tuple<bool, bool> error1 = digitalIn1.getStateOfBit(0);
        std::tuple<bool, bool> warning1 = digitalIn1.getStateOfBit(1);
        std::tuple<bool, bool> warning2 = digitalIn1.getStateOfBit(2);
        std::tuple<bool, bool> remoteMeas = digitalIn1.getStateOfBit(3);

        std::tuple<bool, bool> serviceSwitch = digitalIn1.getStateOfBit(4);
        std::tuple<bool, bool> calibrateSwitch = digitalIn1.getStateOfBit(5);

        
        if(std::get<1>(serviceSwitch) || std::get<1>(calibrateSwitch))
        {
            Core::instance->setBlockMeas(true);
        }
        else
        {
            Core::instance->setBlockMeas(false);
        }

        static bool error1_active = false;
        static bool warning1_active = false;
        static bool warning2_active = false;
        static bool remoteMeas_active = false;

        if(std::get<1>(error1))
        {
            if(!error1_active)
            {
                std::cout << "Error 1 was created: " << std::get<1>(error1) << std::endl; 
                OccurredErrorRepository::instance->logError({
                                                    .errCode = "E001",
                                                    .interface = hardwareInterface::gui,
                                                    .location = __PRETTY_FUNCTION__,
                                                    .data = utils::sys::dump_stacktrace(),
                                                    });
            }
        }
        else
        {
            if(error1_active)
            {
                std::cout << "Error 1 was disabled: " << std::get<1>(error1) << std::endl; 
            }
        }

        if(std::get<1>(warning1))
        {
            if(!warning1_active)
            {
                OccurredWarningRepository::instance->logWarning({
                                                    .warnCode = "W001",
                                                    .interface = hardwareInterface::gui,
                                                    .location = __PRETTY_FUNCTION__,
                                                    .data = utils::sys::dump_stacktrace(),
                                                    });
                std::cout << "Warning 1 was created: " << std::get<1>(warning1) << std::endl; 
            }     
        }
        else
        {
            if(warning1_active)
            {
                std::cout << "Warning 1 was disabled: " << std::get<1>(warning1) << std::endl; 
            }
        }

        if(std::get<1>(warning2))
        {
            if(!warning2_active)
            {
                OccurredWarningRepository::instance->logWarning({
                                                    .warnCode = "W002",
                                                    .interface = hardwareInterface::gui,
                                                    .location = __PRETTY_FUNCTION__,
                                                    .data = utils::sys::dump_stacktrace(),
                                                    });
                std::cout << "Warning 2 was created: " << std::get<1>(warning2) << std::endl; 
            }
        }
        else
        {
            if(warning2_active)
            {
                std::cout << "Warning 2 was disabled: " << std::get<1>(warning2) << std::endl; 
            }
        }

        if(std::get<1>(remoteMeas))
        {
            if(!remoteMeas_active && !Core::instance->getBlockMeas())
            {
                Core::instance->control.claimControl(Gui::instance->controlInterface);
                Core::instance->control.send(Gui::instance->controlInterface, core::control::action::MEASUREMENT_START);
                std::cout << "Measurement was created: " << std::get<1>(remoteMeas) << std::endl; 
            }
        }
        else
        {
            if(remoteMeas_active)
            {
                Core::instance->control.send(Gui::instance->controlInterface, core::control::action::MEASUREMENT_STOP);
                Core::instance->control.releaseControl(Gui::instance->controlInterface);
                std::cout << "Measurement was disabled: " << std::get<1>(remoteMeas) << std::endl; 
            }        
        }

        */
        return true;
    }

    bool handle_websocket_remotecontrol() {
        std::cout << "handle_websocket_remotecontrol" << std::endl;

        return true;
    }

    bool handle_plc_interface() {
        std::cout << "handle_plc_interface" << std::endl;

        //Core::instance->plcInterface->serveNonBlocking();

        return true;
    }

    bool handle_plc_firmware_update_via_uart() {
        std::cout << "handle_plc_firmware_update_via_uart" << std::endl;

        bool ret = false;

        if (Core::instance->asyncHandler->getHandleActive(e_handle_plc_interface)) {
            return false;
        }

        {
            Core::instance->plcSerial->connect();

            auto binary_control_string = Core::instance->plcSerial->readUartBinary(0x03);
            std::string payload;
            for (int i = 1; i < binary_control_string.size() - 1; ++i) {
                payload += (char) binary_control_string[i];
            }
            auto pieces = utils::strings::splitString(payload, ";");
            auto it = pieces.begin();

            std::string version_str = *it;
            std::advance(it, 1);
            std::string timestamp_str = *it;
            std::advance(it, 1);
            std::string bytes_str = *it;
            int bytes = std::stoi(bytes_str);

            auto binary_file = Core::instance->plcSerial->readUartBinaryByteCount(bytes);

            std::string fname = Core::instance->config->getValue("FW_VERSIONS_DIR") + "/" + version_str;
            auto target_file = std::fstream(fname, std::ios::out | std::ios::binary);
            if (target_file) {
                for (const auto &item: binary_file) {
                    target_file << item;
                }
                target_file.close();
                ret = true;
            } else {
                Core::instance->plcSerial->disconnect();
            }

            Core::instance->plcSerial->disconnect();
        }

        Core::instance->asyncHandler->setHandleActive(e_handle_plc_interface, true);

        return ret;
    }
}