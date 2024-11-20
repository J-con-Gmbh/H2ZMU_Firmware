//
// Created by jriessner on 01.09.23.
//

#include "data/db/repositories/MeasurementRepository.h"
#include "data/db/repositories/SensorstateRepository.h"
#include "data/db/repositories/CascadeRepository.h"
#include "data/db/repositories/BottleRepository.h"
#include "data/db/repositories/SensorRepository.h"
#include "data/db/repositories/OccurredErrorRepository.h"
#include "data/db/repositories/OccurredWarningRepository.h"
#include "data/db/repositories/ErrorRepository.h"
#include "data/db/repositories/WarningRepository.h"
#include "utilities/modbus/ModbusDataConversion.h"
#include "interface/modbus/PlcInterface.h"
#include "rgfc/RgfCorr.h"
#include "Core.h"

#include <utility>

int PlcInterface::control = -1;
bool PlcInterface::measurement_start = false;
bool PlcInterface::measurement_stop = false;
std::string PlcInterface::measurement_externalId = "Test";

/**
 * This function extend the ModbusServer::processModbusTelegram function with plc_data access restrictions. <br>
 * If the current access restriction is not matched, a illegal data address exception is returned to the modbus client.
 * @param t
 */
void PlcInterface::processModbusTelegram(mb::telegram t) {

    // One based full register number
    uint16_t reg_nr = t.reg_start + t.reg_offset;

    if (this->registerMap.count(reg_nr)) {
        int id = this->registerMap[reg_nr];
        struct plc_data plcData = this->dataMap[id];
        if (t.function_code <= 0x04) {
            if (this->access_read < plcData.access_read) {
                //modbus_reply_exception(this->ctx, t.req.data(), MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
                return;
            }
        } else {
            if (this->access_write < plcData.access_write) {
                //modbus_reply_exception(this->ctx, t.req.data(), MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
                return;
            }
        }
    }

    ModbusServer::processModbusTelegram(std::move(t));
}

void PlcInterface::updateDeviceState() {
    if (false /* TODO Maybe check access == SERVICE as well, depend on security guideline
                * -> e.G. if Switch and software flag has to be set
                * TODO get service switch current_state */ ) {
        this->access_read == SERVICE;
    }
    if (false /* TODO Maybe check access == CALIBRATION as well, depend on security guideline
                * -> e.G. if Switch and software flag has to be set
                * TODO get calibration switch current_state */ ) {
        this->access_write == CALIBRATION;
    }
}

PlcInterface::PlcInterface(const mb::modbus_config &config) : ModbusServer(config) {
    this->defineDataStructs();
    this->updateDeviceState();
}

void PlcInterface::setup() {

    this->control = Core::instance->control.registerInterface(1);

    switch(this->access_read) {
        case CALIBRATION:
            /// Load all data if clearance is CALIBRATION
            this->loadData(CALIBRATION);
            break;
        case SERVICE:
            /// Load SERVICE and USER data if clearance is SECURITY
            this->loadData(SERVICE);
            break;
        case USER:
            /// only load USER data
            this->loadData(USER);
            break;
        case READONLY:
            break;
    }

    ModbusServer::setup();
}

std::vector<uint16_t> PlcInterface::mb_returnBlank(mb::modbus_register_data *data, const mb::telegram &t) {
    return std::vector<uint16_t>();
}

std::vector<uint16_t> PlcInterface::mb_getCurrentMeasurementId(mb::modbus_register_data *data, const mb::telegram &t) {
    int id = MeasurementRepository::instance->getCurrentId();

    if(id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    return dataTypeToRegisterData<int>(id);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeCount(mb::modbus_register_data *data, const mb::telegram &t) {
    int count = CascadeRepository::instance->getAllCascades().size();

    return dataTypeToRegisterData<int>(count);
}

std::vector<uint16_t> PlcInterface::mb_getLastMeasurementMass(mb::modbus_register_data *data, const mb::telegram &t) {
    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();

    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    return dataTypeToRegisterData<float>(_measurement.amountEndNm3 - _measurement.amountStartNm3);
}

std::vector<uint16_t> PlcInterface::mb_getLastMeasurementStart(mb::modbus_register_data *data, const mb::telegram &t) {
    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();
    
    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    return stringToRegisterData(utils::epoch_time::formatTimestamp(_measurement.timestamp_start));
}

std::vector<uint16_t> PlcInterface::mb_getLastMeasurementEnd(mb::modbus_register_data *data, const mb::telegram &t) {
    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();

    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    return stringToRegisterData(utils::epoch_time::formatTimestamp(_measurement.timestamp_end));
}

std::vector<uint16_t> PlcInterface::mb_getLastMeasurementMassStart(mb::modbus_register_data *data, const mb::telegram &t) {
    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();

    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    return dataTypeToRegisterData<float>(_measurement.amountStartNm3);
}

std::vector<uint16_t> PlcInterface::mb_getLastMeasurementMassEnd(mb::modbus_register_data *data, const mb::telegram &t) {
    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();

    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    return dataTypeToRegisterData<float>(_measurement.amountEndNm3);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeMassDifference(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;

    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();

    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    struct cascadestate start_state;
    struct cascadestate end_state;

    struct vesselstate start_vessel_state = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_start);
    struct vesselstate end_vessel_state = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_end);
    float value = -1;

    for(const auto &_cascadestate: start_vessel_state.fk_cascadestates)
    {
        if(cascade_id == CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade)
        {
            start_state = CascadestateRepository::instance->getCascadestateById(_cascadestate);
        }
    }

    for (const auto &_cascadestate: end_vessel_state.fk_cascadestates)
    {
        if(cascade_id == CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade)
        {
            end_state = CascadestateRepository::instance->getCascadestateById(_cascadestate);
        }
    }

    if(start_state.id != -1 && end_state.id != -1)
    {
        value = start_state.content_kg - end_state.content_kg;
    }

    return dataTypeToRegisterData<float>(value);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeStartMass(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;

    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();
    
    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    struct vesselstate start_vessel_state = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_start);
    struct cascadestate start_state;
    float value = -1;

    for (const auto &_cascadestate: start_vessel_state.fk_cascadestates)
    {
        if(cascade_id == CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade)
        {
            start_state = CascadestateRepository::instance->getCascadestateById(_cascadestate);
        }
    }

    if(start_state.id != -1)
    {
        value = start_state.content_kg;
    }

    return dataTypeToRegisterData<float>(value);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeStartPressure(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;

    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();

    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    struct vesselstate start_vessel_state = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_start);
    struct cascadestate start_state;
    float value = -1;

    for (const auto &_cascadestate: start_vessel_state.fk_cascadestates)
    {
        if(cascade_id == CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade)
        {
            start_state = CascadestateRepository::instance->getCascadestateById(_cascadestate);
        }
    }

    if(start_state.id != -1)
    {
        struct sensorstate sensorstate = SensorstateRepository::instance->getSensorstate(start_state.fk_pressure_upper_sensorstate);
        value = sensorstate.value;
    }
    
    return dataTypeToRegisterData<float>(value);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeStartTemperature(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;

    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();

    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    struct vesselstate start_vessel_state = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_start);
    struct cascadestate start_state;
    float value = -1;

    for (const auto &_cascadestate: start_vessel_state.fk_cascadestates)
    {
        if(cascade_id == CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade)
        {
            start_state = CascadestateRepository::instance->getCascadestateById(_cascadestate);
        }
    }

    if(start_state.id != -1)
    {
        struct sensorstate sensorstate = SensorstateRepository::instance->getSensorstate(start_state.fk_temperature_values[0]);
        value = sensorstate.value;
    }

    return dataTypeToRegisterData<float>(value);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeStartRGFS(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;

    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();

    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    struct vesselstate start_vessel_state = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_start);
    struct cascadestate start_state;
    float value = -1;

    for (const auto &_cascadestate: start_vessel_state.fk_cascadestates)
    {
        if(cascade_id == CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade)
        {
            start_state = CascadestateRepository::instance->getCascadestateById(_cascadestate);
        }
    }
    
    if(start_state.id != -1)
    {
        struct sensorstate sensorstate_temp = SensorstateRepository::instance->getSensorstate(start_state.fk_temperature_values[0]);
        struct sensorstate sensorstate_pressure = SensorstateRepository::instance->getSensorstate(start_state.fk_pressure_upper_sensorstate);

        value = RgfCorr::instance->getNormVolume_m3(start_state.norm_volume, sensorstate_pressure.value, sensorstate_temp.value + 273.15);
    }
    
    return dataTypeToRegisterData<float>(value);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeStartVolumeCorrected(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;

    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();

    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    struct vesselstate start_vessel_state = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_start);
    struct cascadestate start_state;
    float value = -1;

    for (const auto &_cascadestate: start_vessel_state.fk_cascadestates)
    {
        if(cascade_id == CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade)
        {
            start_state = CascadestateRepository::instance->getCascadestateById(_cascadestate);
        }
    }

    if(start_state.id != -1)
    {
        value = start_state.geom_volume_corr;
    }

    return dataTypeToRegisterData<float>(value);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeEndMass(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;

    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();

    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    struct vesselstate end_vessel_state = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_end);
    struct cascadestate end_state;
    float value = -1;

    for (const auto &_cascadestate: end_vessel_state.fk_cascadestates)
    {
        if(cascade_id == CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade)
        {
            end_state = CascadestateRepository::instance->getCascadestateById(_cascadestate);
        }
    }

    if(end_state.id != -1)
    {
        value = end_state.content_kg;
    }

    return dataTypeToRegisterData<float>(value);
}
    
std::vector<uint16_t> PlcInterface::mb_getCascadeEndPressure(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;

    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();

    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    struct vesselstate end_vessel_state = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_end);
    struct cascadestate end_state;
    float value = -1;

    for (const auto &_cascadestate: end_vessel_state.fk_cascadestates)
    {
        if(cascade_id == CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade)
        {
            end_state = CascadestateRepository::instance->getCascadestateById(_cascadestate);
        }
    }

    if(end_state.id != -1)
    {
        struct sensorstate sensorstate = SensorstateRepository::instance->getSensorstate(end_state.fk_pressure_upper_sensorstate);
        value = sensorstate.value;
    }

    return dataTypeToRegisterData<float>(value);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeEndTemperature(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;

    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();

    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    struct vesselstate end_vessel_state = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_end);
    struct cascadestate end_state;
    float value = -1;

    for (const auto &_cascadestate: end_vessel_state.fk_cascadestates)
    {
        if(cascade_id == CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade)
        {
            end_state = CascadestateRepository::instance->getCascadestateById(_cascadestate);
        }
    }

    if(end_state.id != -1)
    {
        struct sensorstate sensorstate = SensorstateRepository::instance->getSensorstate(end_state.fk_temperature_values[0]);
        value = sensorstate.value;
    }
    return dataTypeToRegisterData<float>(value);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeEndRGFS(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;

    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();

    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    struct vesselstate end_vessel_state = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_end);
    struct cascadestate end_state;
    float value = -1;

    for (const auto &_cascadestate: end_vessel_state.fk_cascadestates)
    {
        if(cascade_id == CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade)
        {
            end_state = CascadestateRepository::instance->getCascadestateById(_cascadestate);
        }
    }

    if(end_state.id != -1)
    {
        struct sensorstate sensorstate_temp = SensorstateRepository::instance->getSensorstate(end_state.fk_temperature_values[0]);
        struct sensorstate sensorstate_pressure = SensorstateRepository::instance->getSensorstate(end_state.fk_pressure_upper_sensorstate);

        value = RgfCorr::instance->getNormVolume_m3(end_state.norm_volume, sensorstate_pressure.value, sensorstate_temp.value + 273.15);
    }

    return dataTypeToRegisterData<float>(value);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeEndVolumeCorrected(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;

    struct measurement _measurement = MeasurementRepository::instance->getLastMeasurement();

    if(_measurement.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    struct vesselstate end_vessel_state = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_end);
    struct cascadestate end_state;
    float value = -1;

    for (const auto &_cascadestate: end_vessel_state.fk_cascadestates)
    {
        if(cascade_id == CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade)
        {
            end_state = CascadestateRepository::instance->getCascadestateById(_cascadestate);
        }
    }

    if(end_state.id != -1)
    {
        value = end_state.geom_volume_corr;
    }

    return dataTypeToRegisterData<float>(value);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeMassTotal(mb::modbus_register_data *data, const mb::telegram &t) {
    std::map<int, struct vesselstate> allVesselstates = VesselstateRepository::instance->getAllVesselstates();

    struct vesselstate latest_vesselstate;

    for (const auto &_vesselstate: allVesselstates)
    {
        if(_vesselstate.second.id > latest_vesselstate.id)
        {
            latest_vesselstate = _vesselstate.second;
        }
    }

    return dataTypeToRegisterData<float>(latest_vesselstate.content_kg);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeMass(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;
    std::map<int, struct vesselstate> allVesselstates = VesselstateRepository::instance->getAllVesselstates();
    
    float cascade_mass = 0;

    struct vesselstate latest_vesselstate;

    for (const auto &_vesselstate: allVesselstates)
    {
        if(_vesselstate.second.id > latest_vesselstate.id)
        {
            latest_vesselstate = _vesselstate.second;
        }
    }

    for(const auto &_cascadestate: latest_vesselstate.fk_cascadestates)
    {
        if(CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade == cascade_id)
        {
            cascade_mass = CascadestateRepository::instance->getCascadestateById(_cascadestate).content_kg;
        }
    }

    return dataTypeToRegisterData<float>(cascade_mass);
}

std::vector<uint16_t> PlcInterface::mb_getCascadePressure(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;
    std::map<int, struct vesselstate> allVesselstates = VesselstateRepository::instance->getAllVesselstates();
    struct sensorstate _sensorstate;

    struct vesselstate latest_vesselstate;

    for (const auto &_vesselstate: allVesselstates)
    {
        if(_vesselstate.second.id > latest_vesselstate.id)
        {
            latest_vesselstate = _vesselstate.second;
        }
    }

    for(const auto &_cascadestate: latest_vesselstate.fk_cascadestates)
    {
        if(CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade == cascade_id)
        {
            _sensorstate = SensorstateRepository::instance->getSensorstate(CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_pressure_upper_sensorstate);
        }
    }
    
    return dataTypeToRegisterData<float>(_sensorstate.value);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeTemperature(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;
    std::map<int, struct vesselstate> allVesselstates = VesselstateRepository::instance->getAllVesselstates();
    struct sensorstate _sensorstate;

    struct vesselstate latest_vesselstate;

    for (const auto &_vesselstate: allVesselstates)
    {
        if(_vesselstate.second.id > latest_vesselstate.id)
        {
            latest_vesselstate = _vesselstate.second;
        }
    }

    for(const auto &_cascadestate: latest_vesselstate.fk_cascadestates)
    {
        if(CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade == cascade_id)
        {
            _sensorstate = SensorstateRepository::instance->getSensorstate(CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_temperature_values[0]);
        }
    }
    
    return dataTypeToRegisterData<float>(_sensorstate.value);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeRGFS(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;
    std::map<int, struct vesselstate> allVesselstates = VesselstateRepository::instance->getAllVesselstates();
    struct sensorstate _sensorstate_temp;
    struct sensorstate _sensorstate_pressure;
    float norm_volume = 0;

    struct vesselstate latest_vesselstate;

    for (const auto &_vesselstate: allVesselstates)
    {
        if(_vesselstate.second.id > latest_vesselstate.id)
        {
            latest_vesselstate = _vesselstate.second;
        }
    }

    for(const auto &_cascadestate: latest_vesselstate.fk_cascadestates)
    {
        if(CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade == cascade_id)
        {
            _sensorstate_temp = SensorstateRepository::instance->getSensorstate(CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_temperature_values[0]);
            _sensorstate_pressure = SensorstateRepository::instance->getSensorstate(CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_pressure_upper_sensorstate);
            norm_volume = CascadestateRepository::instance->getCascadestateById(_cascadestate).norm_volume;
        }
    }

    float rfg = RgfCorr::instance->getNormVolume_m3(norm_volume, _sensorstate_pressure.value, _sensorstate_temp.value + 273.15);
    
    return dataTypeToRegisterData<float>(rfg);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeVolumeCorrected(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;
    std::map<int, struct vesselstate> allVesselstates = VesselstateRepository::instance->getAllVesselstates();
    struct sensorstate _sensorstate_temp;
    struct sensorstate _sensorstate_pressure;
    float norm_volume = 0;

    struct vesselstate latest_vesselstate;

    for (const auto &_vesselstate: allVesselstates)
    {
        if(_vesselstate.second.id > latest_vesselstate.id)
        {
            latest_vesselstate = _vesselstate.second;
        }
    }

    for(const auto &_cascadestate: latest_vesselstate.fk_cascadestates)
    {
        if(CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_cascade == cascade_id)
        {
            _sensorstate_temp = SensorstateRepository::instance->getSensorstate(CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_temperature_values[0]);
            _sensorstate_pressure = SensorstateRepository::instance->getSensorstate(CascadestateRepository::instance->getCascadestateById(_cascadestate).fk_pressure_upper_sensorstate);
            norm_volume = CascadestateRepository::instance->getCascadestateById(_cascadestate).norm_volume;
        }
    }

    float rfg = RgfCorr::instance->getNormVolume_m3(norm_volume, _sensorstate_pressure.value, _sensorstate_temp.value + 273.15);
    
    return dataTypeToRegisterData<float>(rfg);
}

std::vector<uint16_t> PlcInterface::mb_getTempSensorCascade(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    struct sensorstate latest_sensorstate;
    std::map<int, struct sensor> allTempsensors = SensorRepository::instance->getAllTempSensors();
    std::map<int, struct sensorstate> allSensorstates = SensorstateRepository::instance->getAllSensorstates();
    struct cascadestate _cascadestate;

    for (const auto &_sensorstate: allSensorstates)
    {
        if(_sensorstate.second.fk_sensor == allTempsensors[it].id)
        {
            if(_sensorstate.second.timestamp > latest_sensorstate.timestamp)
            {
                _cascadestate = CascadestateRepository::instance->getCascadestateById(_sensorstate.second.fk_cstate);
            }
        }
    }

    if(_cascadestate.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    return dataTypeToRegisterData<int>(_cascadestate.fk_cascade);
}

std::vector<uint16_t> PlcInterface::mb_getTempValue(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    struct sensorstate latest_sensorstate;
    std::map<int, struct sensor> allTempsensors = SensorRepository::instance->getAllTempSensors();

    std::map<int, struct sensorstate> allSensorstates = SensorstateRepository::instance->getAllSensorstates();

    for (const auto &_sensorstate: allSensorstates)
    {
        if(_sensorstate.second.fk_sensor == allTempsensors[it].id)
        {
            if(_sensorstate.second.timestamp > latest_sensorstate.timestamp)
            {
                latest_sensorstate = _sensorstate.second;
            }
        }
    }

    if(latest_sensorstate.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    return dataTypeToRegisterData<float>(latest_sensorstate.value);
}


// TODO: 2000 - 2005  

std::vector<uint16_t> PlcInterface::mb_getCascadeId(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = -1;
    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        int new_id = registerDataToSimpleType<int>(regData);
        cascade_id = CascadeRepository::instance->getCascadeById(it).id;
        cascade_id = CascadeRepository::instance->setCascadeId(cascade_id, new_id).id;
    }
    
    cascade_id = CascadeRepository::instance->getCascadeById(it).id;

    return dataTypeToRegisterData<int>(cascade_id);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeCountBottles(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;
    std::map<int, struct bottle> allBottles = BottleRepository::instance->getAllBottles();

    int bottle_count = 0;

    for (const auto &_bottle: allBottles)
    {
        if(_bottle.second.fk_cascade == cascade_id)
        {
            bottle_count++;
        }
    }

    return dataTypeToRegisterData<int>(bottle_count);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeVolume0Pressure(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;
    std::map<int, struct bottle> allBottles = BottleRepository::instance->getAllBottles();

    float vol_0_add = -1;

    for (const auto &_bottle: allBottles)
    {
        if(_bottle.second.fk_cascade == cascade_id)
        {
            vol_0_add = vol_0_add + _bottle.second.vol_0;
        }
    }

    return dataTypeToRegisterData<float>(vol_0_add);
}

std::vector<uint16_t> PlcInterface::mb_getCascadePressureSensorId(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        int fk_pressure = registerDataToSimpleType<int>(regData);
        CascadeRepository::instance->setCascadePressureSensor(it, fk_pressure);
    }

    struct sensor _sensor = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper);

    return dataTypeToRegisterData<int>(_sensor.id);
}

std::vector<uint16_t> PlcInterface::mb_getCascadePressureSensorSerial(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    
    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        std::string serial = registerDataToString(regData);
        int sensor_id = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper).id;
        SensorRepository::instance->setSensorSerial(sensor_id, serial);
    }

    struct sensor _sensor = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper);

    return stringToRegisterData(_sensor.serialnumber);
}

std::vector<uint16_t> PlcInterface::mb_getCascadePressureSensorLowerLimitManufacturer(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float limit = registerDataToSimpleType<float>(regData);
        int sensor_id = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper).id;
        SensorRepository::instance->setSensorLowerLimitManufacturer(sensor_id, limit);
    }

    struct sensor _sensor = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper);


    if(_sensor.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    return dataTypeToRegisterData<float>(_sensor.lowermeasurelimit_manufacturer);
}

std::vector<uint16_t> PlcInterface::mb_getCascadePressureSensorUpperLimitManufacturer(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float limit = registerDataToSimpleType<float>(regData);
        int sensor_id = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper).id;
        SensorRepository::instance->setSensorUpperLimitManufacturer(sensor_id, limit);
    }

    struct sensor _sensor = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper);

    if(_sensor.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    return dataTypeToRegisterData<float>(_sensor.uppermeasurelimit_manufacturer);
}

std::vector<uint16_t> PlcInterface::mb_getCascadePressureSensorLowerLimitUser(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float limit = registerDataToSimpleType<float>(regData);
        int sensor_id = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper).id;
        SensorRepository::instance->setSensorLowerLimitUser(sensor_id, limit);
    }
    
    struct sensor _sensor = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper);

    if(_sensor.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    return dataTypeToRegisterData<float>(_sensor.lowermeasurelimit_user);
}

std::vector<uint16_t> PlcInterface::mb_getCascadePressureSensorUpperLimitUser(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float limit = registerDataToSimpleType<float>(regData);
        int sensor_id = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper).id;
        SensorRepository::instance->setSensorUpperLimitUser(sensor_id, limit);
    }

    struct sensor _sensor = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper);

    if(_sensor.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    return dataTypeToRegisterData<float>(_sensor.uppermeasurelimit_user);
}

std::vector<uint16_t> PlcInterface::mb_getCascadePressureSensorCalibrationDate(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        std::string calibratio_date = registerDataToString(regData);
        int sensor_id = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper).id;
        SensorRepository::instance->setSensorCalibrationDate(sensor_id, calibratio_date);
    }

    struct sensor _sensor = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper);

    if(_sensor.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    return stringToRegisterData(utils::epoch_time::formatTimestamp(_sensor.calibration_date));
}

std::vector<uint16_t> PlcInterface::mb_getCascadePressureSensorCorrectionM(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float correction = registerDataToSimpleType<float>(regData);
        int sensor_id = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper).id;
        SensorRepository::instance->setSensorCorrectionM(sensor_id, correction);
    }

    struct sensor _sensor = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper);

    if(_sensor.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    return dataTypeToRegisterData<float>(_sensor.correction_m);
}

std::vector<uint16_t> PlcInterface::mb_getCascadePressureSensorCorrectionB(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float correction = registerDataToSimpleType<float>(regData);
        int sensor_id = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper).id;
        SensorRepository::instance->setSensorCorrectionB(sensor_id, correction);
    }

    struct sensor _sensor = SensorRepository::instance->getSensorById(CascadeRepository::instance->getCascadeById(it).fk_pressure_sensor_upper);

    if(_sensor.id == -1)
    {
        std::vector<uint16_t> empty;
        return empty;
    }

    return dataTypeToRegisterData<float>(_sensor.correction_b);
}

std::vector<uint16_t> PlcInterface::mb_getCascadeMaxPressure(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    uint8_t cascade_id = CascadeRepository::instance->getCascadeById(it).id;
    std::map<int, struct bottle> allBottles = BottleRepository::instance->getAllBottles();

    float cascade_max_operating_pressure = 1000000;

    for (const auto &_bottle: allBottles)
    {
        if(_bottle.second.fk_cascade == cascade_id)
        {
            if(cascade_max_operating_pressure > _bottle.second.max_operating_pressure)
            {
                cascade_max_operating_pressure = _bottle.second.max_operating_pressure;
            }
        }
    }

    return dataTypeToRegisterData<float>(cascade_max_operating_pressure);
}

std::vector<uint16_t> PlcInterface::mb_getTempSensorId(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    struct sensor _sensor = SensorRepository::instance->getSensorById(it);
    std::map<int, struct sensor> allTempsensors = SensorRepository::instance->getAllTempSensors();

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        int new_id = registerDataToSimpleType<int>(regData);
        _sensor = SensorRepository::instance->setSensorId(allTempsensors[it].id, new_id);
    }

    return dataTypeToRegisterData<int>(_sensor.id);
}

std::vector<uint16_t> PlcInterface::mb_getTempSensorBottle(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    struct bottle _bottle;
    std::map<int, struct sensor> allTempsensors = SensorRepository::instance->getAllTempSensors();

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        int new_id = registerDataToSimpleType<int>(regData);
        _bottle = BottleRepository::instance->setBottleSensor(new_id, it);
    }

    std::map<int, struct bottle> allBottles = BottleRepository::instance->getAllBottles();

    for (const auto &_bottle: allBottles)
    {
        if(_bottle.second.fk_sensor == allTempsensors[it].id)
        {
            return dataTypeToRegisterData<int>(_bottle.second.id);
        }
    }

    return dataTypeToRegisterData<int>(-1);
}

std::vector<uint16_t> PlcInterface::mb_getTempSensorSerial(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    std::map<int, struct sensor> allTempsensors = SensorRepository::instance->getAllTempSensors();

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        std::string new_serial = registerDataToString(regData);
        SensorRepository::instance->setSensorSerial(allTempsensors[it].id, new_serial);
    }

    return stringToRegisterData(allTempsensors[it].serialnumber);
}

std::vector<uint16_t> PlcInterface::mb_getTempSensorLowerLimitManufacturer(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    std::map<int, struct sensor> allTempsensors = SensorRepository::instance->getAllTempSensors();

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float new_limit = registerDataToSimpleType<float>(regData);
        SensorRepository::instance->setSensorLowerLimitManufacturer(allTempsensors[it].id, new_limit);
    }

    return dataTypeToRegisterData<float>(SensorRepository::instance->getSensorById(allTempsensors[it].id).lowermeasurelimit_manufacturer);
}

std::vector<uint16_t> PlcInterface::mb_getTempSensorUpperLimitManufacturer(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    std::map<int, struct sensor> allTempsensors = SensorRepository::instance->getAllTempSensors();

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float new_limit = registerDataToSimpleType<float>(regData);
        SensorRepository::instance->setSensorUpperLimitManufacturer(allTempsensors[it].id, new_limit);
    }

    return dataTypeToRegisterData<float>(SensorRepository::instance->getSensorById(allTempsensors[it].id).uppermeasurelimit_manufacturer);
}

std::vector<uint16_t> PlcInterface::mb_getTempSensorLowerLimitUser(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    std::map<int, struct sensor> allTempsensors = SensorRepository::instance->getAllTempSensors();

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float new_limit = registerDataToSimpleType<float>(regData);
        SensorRepository::instance->setSensorLowerLimitUser(allTempsensors[it].id, new_limit);
    }

    return dataTypeToRegisterData<float>(SensorRepository::instance->getSensorById(allTempsensors[it].id).lowermeasurelimit_user);
}

std::vector<uint16_t> PlcInterface::mb_getTempSensorUpperLimitUser(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    std::map<int, struct sensor> allTempsensors = SensorRepository::instance->getAllTempSensors();

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float new_limit = registerDataToSimpleType<float>(regData);
        SensorRepository::instance->setSensorUpperLimitUser(allTempsensors[it].id, new_limit);
    }

    return dataTypeToRegisterData<float>(SensorRepository::instance->getSensorById(allTempsensors[it].id).uppermeasurelimit_user);
}

std::vector<uint16_t> PlcInterface::mb_getTempSensorCalibrationDate(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    std::map<int, struct sensor> allTempsensors = SensorRepository::instance->getAllTempSensors();

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        std::string new_calibration_date = registerDataToString(regData);
        SensorRepository::instance->setSensorCalibrationDate(allTempsensors[it].id, new_calibration_date);
    }

    return stringToRegisterData(utils::epoch_time::formatTimestamp(SensorRepository::instance->getSensorById(allTempsensors[it].id).calibration_date));
}

std::vector<uint16_t> PlcInterface::mb_getTempSensorCorrectionM(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    std::map<int, struct sensor> allTempsensors = SensorRepository::instance->getAllTempSensors();

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float correction = registerDataToSimpleType<float>(regData);
        SensorRepository::instance->setSensorCorrectionM(allTempsensors[it].id, correction);
    }

    return dataTypeToRegisterData<float>(SensorRepository::instance->getSensorById(allTempsensors[it].id).correction_m);
}

std::vector<uint16_t> PlcInterface::mb_getTempSensorCorrectionB(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    std::map<int, struct sensor> allTempsensors = SensorRepository::instance->getAllTempSensors();

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float correction = registerDataToSimpleType<float>(regData);
        SensorRepository::instance->setSensorCorrectionB(allTempsensors[it].id, correction);
    }

    return dataTypeToRegisterData<float>(SensorRepository::instance->getSensorById(allTempsensors[it].id).correction_b);
}

std::vector<uint16_t> PlcInterface::mb_getBottleId(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        int new_id = registerDataToSimpleType<int>(regData);
        BottleRepository::instance->setBottleId(it, new_id);
    }

    return dataTypeToRegisterData<int>(BottleRepository::instance->getBottleById(it).id);
}

std::vector<uint16_t> PlcInterface::mb_getBottleCascade(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        int cascade_id = registerDataToSimpleType<int>(regData);
        BottleRepository::instance->setBottleCascade(it, cascade_id);
    }

    return dataTypeToRegisterData<int>(BottleRepository::instance->getBottleById(it).fk_cascade);
}

std::vector<uint16_t> PlcInterface::mb_getBottleSerial(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        std::string new_serial = registerDataToString(regData);
        BottleRepository::instance->setBottleSerial(it, new_serial);
    }

    return stringToRegisterData(BottleRepository::instance->getBottleById(it).serialnumber);
}

std::vector<uint16_t> PlcInterface::mb_getBottlePosRow(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    return dataTypeToRegisterData<int>(BottleRepository::instance->getBottleById(it).fk_cascade);
}

std::vector<uint16_t> PlcInterface::mb_getBottlePosColumn(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    return dataTypeToRegisterData<int>(BottleRepository::instance->getBottleById(it).fk_cascade);
}

std::vector<uint16_t> PlcInterface::mb_getBottleManufacturer(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        std::string manufacturer = registerDataToString(regData);
        BottleRepository::instance->setBottleManufacturer(it, manufacturer);
    }

    return stringToRegisterData(BottleRepository::instance->getBottleById(it).manufacturer);
}

std::vector<uint16_t> PlcInterface::mb_getBottleBuiltYear(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        int builtyear = registerDataToSimpleType<int>(regData);
        BottleRepository::instance->setBottleBuiltyear(it, builtyear);
    }

    return dataTypeToRegisterData<int>(BottleRepository::instance->getBottleById(it).builtyear);
}

std::vector<uint16_t> PlcInterface::mb_getBottlePressure0(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float pressure_0 = registerDataToSimpleType<float>(regData);
        BottleRepository::instance->setBottlePressure0(it, pressure_0);
    }
    
    return dataTypeToRegisterData<float>(BottleRepository::instance->getBottleById(it).pressure_0);
}

std::vector<uint16_t> PlcInterface::mb_getBottleVolume0(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float vol_0 = registerDataToSimpleType<float>(regData);
        BottleRepository::instance->setBottleVol0(it, vol_0);
    }

    return dataTypeToRegisterData<float>(BottleRepository::instance->getBottleById(it).vol_0);
}

std::vector<uint16_t> PlcInterface::mb_getBottlePressureRef(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float pressure_ref = registerDataToSimpleType<float>(regData);
        BottleRepository::instance->setBottlePressureRef(it, pressure_ref);
    }

    return dataTypeToRegisterData<float>(BottleRepository::instance->getBottleById(it).pressure_ref);
}

std::vector<uint16_t> PlcInterface::mb_getBottleVolumeRef(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        float vol_ref = registerDataToSimpleType<float>(regData);
        BottleRepository::instance->setBottlePressure0(it, vol_ref);
    }

    return dataTypeToRegisterData<float>(BottleRepository::instance->getBottleById(it).vol_ref);
}


std::vector<uint16_t> PlcInterface::mb_getErrorId(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    return dataTypeToRegisterData<int>(OccurredErrorRepository::instance->getOccurredErrorById(it).id);
}

std::vector<uint16_t> PlcInterface::mb_getErrorCode(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    std::map<int, struct error> allErrors = ErrorRepository::instance->getAllErrors();

    struct error _error = ErrorRepository::instance->getErrorById(OccurredErrorRepository::instance->getOccurredErrorById(it).fk_error);
    return stringToRegisterData(_error.errCode);
}

std::vector<uint16_t> PlcInterface::mb_getErrorDate(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    return stringToRegisterData(OccurredErrorRepository::instance->getOccurredErrorById(it).occurred_timestamp);
}

std::vector<uint16_t> PlcInterface::mb_getErrorResolved(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        std::string resolved_date = registerDataToString(regData);
        OccurredErrorRepository::instance->setErrorResolved(it, resolved_date);
    }

    return stringToRegisterData(OccurredErrorRepository::instance->getOccurredErrorById(it).resolved_timestamp);
}

std::vector<uint16_t> PlcInterface::mb_getWarningId(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    return dataTypeToRegisterData<int>(OccurredWarningRepository::instance->getOccurredWarningById(it).id);
}

std::vector<uint16_t> PlcInterface::mb_getWarningCode(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);
    std::map<int, struct warning> allWarnings = WarningRepository::instance->getAllWarnings();

    struct warning _warning = WarningRepository::instance->getWarningById(OccurredWarningRepository::instance->getOccurredWarningById(it).fk_warning);

    return stringToRegisterData(_warning.warnCode);
}

std::vector<uint16_t> PlcInterface::mb_getWarningDate(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    return stringToRegisterData(OccurredWarningRepository::instance->getOccurredWarningById(it).occurred_timestamp);
}

std::vector<uint16_t> PlcInterface::mb_getWarningResolved(mb::modbus_register_data *data, const mb::telegram &t) {
    uint8_t it = std::stoi(data->function_pointer.ptr_arg);

    if (t.function_code > 0x04)
    {
        std::vector<uint16_t> regData( (t.data.size()/2) + (t.data.size()%2), 0x0000 );
        for (int i = 0; i < t.data.size(); ++i)
        {
            unsigned char item = t.data[i];
            regData[i/2] = regData[i/2] | (item << ( (i % 2) * 8));
        }

        std::string resolved_date = registerDataToString(regData);
        OccurredWarningRepository::instance->setWarningResolved(it, resolved_date);
    }
    
    return stringToRegisterData(OccurredWarningRepository::instance->getOccurredWarningById(it).resolved_timestamp);
}


std::string PlcInterface::dumpRegisterMapping() {
    std::stringstream sstream;
    for (const auto &item: this->dataMap) {
        struct plc_data data = item.second;
        sstream << "Nr: " << data.nr << "\t";

        if(data.nr < 999 && data.nr > -100){
            sstream << "\t";
        } 

        sstream << data.short_desc;

        if(data.short_desc.length() < 8){
            sstream << "\t";
        } 
        sstream << " \t[ ";

        switch (data.register_data.type) {
            case mb::VALUE:
                sstream << "VALUE ";
                break;
            case mb::POINTER:
                sstream << "POINTER";
                break;
            case mb::FUNCTION_POINTER:
                sstream << "FUNCTION_POINTER -> ";
                int ts = (int) data.register_data.function_pointer.type_size;
                switch (data.register_data.function_pointer.type) {
                    case mb::INT16:
                        sstream << "INT16:" << ts;
                        break;
                    case mb::INT32:
                        sstream << "INT32:" << ts;
                        break;
                    case mb::FLOAT32:
                        sstream << "FLOAT32:" << ts;
                        break;
                    case mb::BOOL:
                        sstream << "BOOL:" << ts;
                        break;
                    case mb::STRING:
                        sstream << "STRING:" << ts;
                        break;
                    case mb::CHAR:
                        sstream << "CHAR:" << ts;
                        break;
                }
                break;
        }
        sstream << " ] \t";
        switch (data.register_data.type) {
            case mb::FUNCTION_POINTER:
                switch (data.register_data.function_pointer.type) {
                    case mb::BOOL:
                            sstream << "\t";
                        break;
                    case mb::CHAR:
                            sstream << "\t";
                        break;
                }
                break;
        }
    
        sstream << "Register: " << data.register_data.address << "/" << data.register_data.total_register_length << "\t";

        switch (data.register_data.direction) {

            case mb::INPUT:
                sstream << " INPUT";
                sstream << "\t";
                break;
            case mb::OUTPUT:
                sstream << " OUTPUT";
                sstream << "\t";
                break;
            case mb::BIDIRECTIONAL:
                sstream << " BIDIRECTIONAL";
                break;
        }
        sstream << "\t";

        if(data.register_data.type == mb::FUNCTION_POINTER)
        {
            const mb::telegram t = {};
            const mb::modbus_function_pointer &function_pointer = data.register_data.function_pointer;
            auto a = function_pointer.ptr(&data.register_data, t);

            sstream << "Function Parameter: " << data.register_data.function_pointer.ptr_arg << "\t";

            if(data.register_data.function_pointer.type == mb::STRING)
            {
                std::string result = registerDataToStringBigEndian(a);
                sstream << "Function Result: " << result << "\t";
            }
            else if(data.register_data.function_pointer.type == mb::BOOL)
            {
                std::reverse(a.begin(), a.end());
                bool result = registerDataToSimpleType<bool>(a);
                if(result)
                {
                    sstream << "Function Result: " << "TRUE" << "\t";
                }
                else
                {
                    sstream << "Function Result: " << "FALSE" << "\t";
                }
            }
            else if(data.register_data.function_pointer.type == mb::INT32)
            {
                std::reverse(a.begin(), a.end());
                int result = registerDataToSimpleType<int>(a);
                sstream << "Function Result: " << result << "\t";
            }
            else if(data.register_data.function_pointer.type == mb::FLOAT32)
            {
                std::reverse(a.begin(), a.end());
                float result = registerDataToSimpleType<float>(a);
                sstream << "Function Result: " << result << "\t";
            }
            else if(data.register_data.function_pointer.type == mb::INT16)
            {
                std::reverse(a.begin(), a.end());
                int result = registerDataToSimpleType<int>(a);
                sstream << "Function Result: " << result << "\t";
            }
            else if(data.register_data.function_pointer.type == mb::CHAR)
            {
                std::reverse(a.begin(), a.end());
                int result = registerDataToSimpleType<char>(a);
                sstream << "Function Result: " << result << "\t";
            }
        }

        sstream << "\n";

    }
    return sstream.str();
}