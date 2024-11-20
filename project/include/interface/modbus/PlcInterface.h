//
// Created by jriessner on 01.09.23.
//

#ifndef H2ZMU_V1_PLCINTERFACE_H
#define H2ZMU_V1_PLCINTERFACE_H

#include "modbus_interface.h"
#include "datatypes.h"

class PlcInterface : public mb::ModbusServer {
private:
    static int control;

    enum usergroup {
        USER = 1,
        SERVICE = 2,
        CALIBRATION = 3,
        READONLY = 4
    };

    enum datarw {
        READ = 1,
        WRITE = 2,
        READWRITE = 3
    };

    struct plc_data {
        int nr = -1;
        std::string short_desc;
        std::string long_desc;
        enum usergroup access_read;
        enum usergroup access_write;
        mb::modbus_register_data register_data;
    };

    usergroup access_read = usergroup::USER;
    usergroup access_write = usergroup::USER;
    std::map<int, struct plc_data> dataMap;
    std::map<uint16_t, int> registerMap;


    /**
     *  This function predefines all available modbus data.
     */
    void defineDataStructs();

    /**
     * This function loads user accessible data.
     */
    void loadData(usergroup accessClearance);

    /**
     * This function unloads accessible data filtered by accessClearance user group
     */
    void unloadData(usergroup accessClearance);

protected:
    /**
     * This function overrides the function of the ModbusServer class, and adds access_read control.
     * @param telegram t
     */
    void processModbusTelegram(mb::telegram t) override;

    static std::string measurement_externalId;

public:
    static bool measurement_start;
    static bool measurement_stop;


    explicit PlcInterface(const mb::modbus_config &config);

    /**
     * This function checks Service and Calibration switches, and updates the available data according to the switch states.
     */
    void updateDeviceState();

    /**
     *
     */
    void setAccessClearance(usergroup accessClearanceRead, usergroup accessClearanceWrite);

    void setup() override;

    std::string dumpRegisterMapping();


    /**
     * This function wraps the utils::epoch_time::getTimestamp function in suitable function <br>
     * to be used in the function pointer implementation in the modbus server class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_returnBlank(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps the utils::epoch_time::getTimestamp function in suitable function <br>
     * to be used in the function pointer implementation in the modbus server class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_startMeasurement(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps the utils::epoch_time::getTimestamp function in suitable function <br>
     * to be used in the function pointer implementation in the modbus server class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_stopMeasurement(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps the utils::epoch_time::getTimestamp function in suitable function <br>
     * to be used in the function pointer implementation in the modbus server class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getDaytime(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps the utils::epoch_time::getTimestamp function in suitable function <br>
     * to be used in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getDate(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_readWriteParameter(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCurrentMeasurementId(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeCount(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getLastMeasurementMass(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getLastMeasurementStart(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getLastMeasurementEnd(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getLastMeasurementMassStart(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getLastMeasurementMassEnd(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeMassDifference(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeStartMass(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeStartPressure(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeStartTemperature(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeStartRGFS(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeStartVolumeCorrected(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeEndMass(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeEndPressure(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeEndTemperature(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeEndRGFS(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeEndVolumeCorrected(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getBottleId(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getBottleCascade(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getBottleSerial(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getBottlePosRow(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getBottlePosColumn(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getBottleManufacturer(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getBottleBuiltYear(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getBottlePressure0(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getBottleVolume0(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getBottlePressureRef(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getBottleVolumeRef(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeMassTotal(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeMass(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadePressure(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeTemperature(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeRGFS(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeVolumeCorrected(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getTempSensorCascade(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getTempValue(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeId(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeCountBottles(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeVolume0Pressure(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadePressureSensorId(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadePressureSensorSerial(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadePressureSensorLowerLimitManufacturer(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadePressureSensorUpperLimitManufacturer(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadePressureSensorLowerLimitUser(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadePressureSensorUpperLimitUser(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadePressureSensorCalibrationDate(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadePressureSensorCorrectionM(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadePressureSensorCorrectionB(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getCascadeMaxPressure(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getTempSensorId(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getTempSensorBottle(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getTempSensorSerial(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getTempSensorLowerLimitManufacturer(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getTempSensorUpperLimitManufacturer(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getTempSensorLowerLimitUser(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getTempSensorUpperLimitUser(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getTempSensorCalibrationDate(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getTempSensorCorrectionM(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getTempSensorCorrectionB(mb::modbus_register_data *data, const mb::telegram &t = {});

        /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getErrorId(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getErrorCode(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getErrorDate(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getErrorResolved(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getWarningId(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getWarningCode(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getWarningDate(mb::modbus_register_data *data, const mb::telegram &t = {});
    
    /**
     * This function wraps read-write functionality of parameter in a suitable function to be used <br>
     * in the function pointer implementation of the ModbusServer class
     * @param modbus_register_data that holds the function pointer
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_getWarningResolved(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function accepts a <b>struct</b> sensor encoded in modbus register data, <br>
     * parses it and stores it for later validation.
     * @param modbus_register_data data that holds the function pointer
     * @param telegram t that holds the encoded sensor struct
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_receiveVesselData_Sensor(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function accepts a <b>struct</b> cascade encoded in modbus register data, <br>
     * parses it and stores it for later validation.
     * @param modbus_register_data data that holds the function pointer
     * @param telegram t that holds the encoded cascade struct
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_receiveVesselData_Cascade(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function accepts a <b>struct</b> bottle encoded in modbus register data, <br>
     * parses it and stores it for later validation.
     * @param modbus_register_data data that holds the function pointer
     * @param telegram t that holds the encoded bottle struct
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_receiveVesselData_Bottle(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function accepts a <b>struct</b> bottle encoded in modbus register data, <br>
     * parses it and stores it for later validation.
     * @param modbus_register_data data that holds the function pointer
     * @param telegram t that holds the encoded bottle struct
     * @return <b>register_data</b> that's going to be set to the libmodbus
     * registers and returned to the modbus client
     */
    static std::vector<uint16_t> mb_receiveVesselData_Validate(mb::modbus_register_data *data, const mb::telegram &t = {});

    /**
     * This function receives a raw binary file<br>
     * @param modbus_register_data data that holds the function pointer
     * @param telegram t
     * @return <b>register_data</b>
     */
    static std::vector<uint16_t> mb_receiveFirmwareUpdate(mb::modbus_register_data *data, const mb::telegram &t = {});


};


#endif //H2ZMU_V1_PLCINTERFACE_H
