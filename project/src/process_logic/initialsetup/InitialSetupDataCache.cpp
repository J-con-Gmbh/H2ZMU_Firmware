//
// Created by jriessner on 07.10.23.
//

#include "process_logic/initialsetup/InitialSetupDataCache.h"

#include "data/db/repositories/HardwareProtocolRepository.h"
#include "data/db/repositories/OccurredErrorRepository.h"
#include "data/db/repositories/SensorRepository.h"


struct setup_data InitialSetupDataCache::setupData = { .sensors=std::vector<struct sensor>(), .cascades=std::vector<struct cascade>(), .bottles=std::vector<struct bottle>() };

void InitialSetupDataCache::add_sensor(struct sensor _sensor) {
    InitialSetupDataCache::setupData.sensors.push_back(_sensor);
}

void InitialSetupDataCache::add_cascade(struct cascade _cascade) {
    InitialSetupDataCache::setupData.cascades.emplace_back(_cascade);
}

void InitialSetupDataCache::add_bottle(const struct bottle& _bottle) {
    InitialSetupDataCache::setupData.bottles.emplace_back(_bottle);
}


struct setup_data InitialSetupDataCache::get_setupdata(){
    return setupData;
}

bool InitialSetupDataCache::validate() {

    bool ret_error = false;

    std::map<int, char> sensor_ids;
    std::map<int, char> bottle_ids;
    std::map<int, char> cascade_ids;

    std::map<int, std::map<int, char>> type_order;
    std::map<int, std::map<int, char>> cascade_order;
    std::map<std::string, char> sensor_serialnrs;
    std::map<std::string, char> bottle_serialnrs;

    std::map<int, struct sensor> map_sensors;
    std::map<int, struct cascade> map_cascades;
    std::map<int, struct bottle> map_bottles;


    for (const auto &sensor: setupData.sensors) {
        map_sensors[sensor.id] = sensor;
    }
    for (const auto &cascade: setupData.cascades) {
        map_cascades[cascade.id] = cascade;
    }
    for (const auto &bottle: setupData.bottles) {
        map_bottles[bottle.id] = bottle;
    }


    for (const auto &sensor: setupData.sensors) {
        std::string error;

        if (sensor.id < 0 )
            error += "invalid sensor id ( " + std::to_string(sensor.id) + " ); ";

        if ( sensor_ids.count(sensor.id) )
            error += "duplicate sensor id ( " + std::to_string(sensor.id) + " ); ";
        sensor_ids[sensor.id] = 0x00;

        if (sensor.type <= 0)
            error += "invalid sensor type ( " + std::to_string(sensor.type) + " ); ";

        if (sensor.type_order <= 0 )
            error += "invalid sensor type ( " + std::to_string(sensor.type) + " ); ";

        if ( type_order[sensor.type].count(sensor.type_order) )
            error += "duplicate sensor type order ( " + std::to_string(sensor.type) + ", " + std::to_string(sensor.type_order) + " ); ";

        if ( sensor.serialnumber.empty() )
            error += "empty serial number; ";

        if ( sensor_serialnrs.count(sensor.serialnumber) )
            error += "duplicate serial number ( " + sensor.serialnumber + " ); ";

        if (sensor.uppermeasurelimit_manufacturer > 10000.0 || sensor.uppermeasurelimit_manufacturer < -1.0)
            error += "upper measure limit invalid 10000 > x > 0 ( " + std::to_string(sensor.uppermeasurelimit_manufacturer) + " ); ";

        if (sensor.uppermeasurelimit_manufacturer > 10000.0 || sensor.uppermeasurelimit_manufacturer < -10000.0)
            error += "upper measure limit invalid: 10000 > x > 0 ( " + std::to_string(sensor.uppermeasurelimit_manufacturer) + " ); ";

        if (sensor.fk_hardwareprotocol < 0)
            error += "invalid hardware protocol forein key ( " + std::to_string(sensor.fk_hardwareprotocol) + " ); ";

        if (sensor.hardwareprotocol_address < 0)
            error += "invalid hardware protocol address ( " + std::to_string(sensor.hardwareprotocol_address) + " ); ";

        if (sensor.offset > 10000.0 || sensor.offset < -1.0)
            error += "offset invalid: 10000 > offset > -1 ( " + std::to_string(sensor.offset) + " ); ";

        if ( HardwareProtocolRepository::instance->getProtocolById(sensor.fk_hardwareprotocol).id < 0 )
            error += "invalid hardware protocol forein key, no protocol found for: " + std::to_string(sensor.fk_hardwareprotocol) + "; ";

        if (!error.empty()) {
            OccurredErrorRepository::instance->logError({
                                                                .errCode = "E100",
                                                                .interface = hardwareInterface::runtime,
                                                                .location = __PRETTY_FUNCTION__,
                                                                .data = error,
                                                        });
            ret_error = true;
        }

    }

    for (const auto &cascade: setupData.cascades) {
        std::string error;

        if (cascade.id < 0 )
            error += "invalid cascade id ( " + std::to_string(cascade.id) + " ); ";

        if (cascade.fk_pressure_sensor_upper < 0)
            error += "invalid upper pressure sensor foreign key ( " + std::to_string(cascade.fk_pressure_sensor_upper) + " ); ";

        /* fk_pressure_sensor_lower not mandatory
         * if (cascade.fk_pressure_sensor_lower < 0 )
            error += "invalid lower pressure sensor foreign key ( " + std::to_string(cascade.fk_pressure_sensor_lower) + " ); ";
        */

        if ( map_sensors.count(cascade.fk_pressure_sensor_upper) && map_sensors.at(cascade.fk_pressure_sensor_upper).id < 0 )
            error += "invalid upper pressure sensor foreign key, no sensor found for: " + std::to_string(cascade.fk_pressure_sensor_lower) + "; ";

        if (!error.empty()) {
            OccurredErrorRepository::instance->logError({
                                                                .errCode = "E101",
                                                                .interface = hardwareInterface::runtime,
                                                                .location = __PRETTY_FUNCTION__,
                                                                .data = error,
                                                        });
            ret_error = true;
        }
    }

    for (const auto &bottle: setupData.bottles) {
        std::string error;

        if (bottle.id < 0)
            error += "invalid bottle id ( " + std::to_string(bottle.id) + " ); ";

        if ( bottle_ids.count(bottle.id) )
            error += "duplicate bottle id ( " + std::to_string(bottle.id) + " ); ";
        bottle_ids[bottle.id] = 0x00;


        if (bottle.serialnumber.empty())
            error += "empty serial number; ";

        if (bottle_serialnrs.count(bottle.serialnumber))
            error += "duplicate serial number ( " + bottle.serialnumber + " ); ";

        if (bottle.fk_cascade < 0)
            error += "invalid cascade foreign key ( " + std::to_string(bottle.fk_cascade) + " ); ";

        if (bottle.cascade_order < 0)
            error += "invalid cascade order ( " + std::to_string(bottle.cascade_order) + " ); ";

        if (cascade_order.count(bottle.cascade_order))
            error += "duplicate cascade order ( " + std::to_string(bottle.fk_cascade) + ", " +
                     std::to_string(bottle.cascade_order) + " ); ";

        // fk_sensor not mandatory
        if (bottle.fk_sensor > 0) {
            if (map_sensors.count(bottle.fk_sensor)) {
                if (map_sensors.at(bottle.fk_sensor).type != 1) {
                    error += "invalid fk_sensor ( " + std::to_string(bottle.fk_sensor) + " ), sensor not type temp; ";
                }
            } else {
                error += "invalid fk_sensor ( " + std::to_string(bottle.fk_sensor) + " ); ";
            }
        }

        if (bottle.tara <= 0)
            error += "invalid tara weight ( " + std::to_string(bottle.tara) + " ); ";

        // manufacturer not mandatory

        // boultyear not mandatory

        // nextcheck not mandatory

        if (bottle.vol_0 <= 0)
            error += "invalid null volume ( " + std::to_string(bottle.vol_0) + " ); ";

        if (bottle.pressure_0 <= 0)
            error += "invalid null volume ( " + std::to_string(bottle.vol_0) + " ); ";

        if (bottle.vol_ref < bottle.vol_0)
            error += "invalid null volume ( " + std::to_string(bottle.vol_0) + " ); ";

        if (bottle.pressure_ref < bottle.pressure_0)
            error += "invalid null volume ( " + std::to_string(bottle.vol_0) + " ); ";

        if (!map_cascades.count(bottle.fk_cascade))
            error += "invalid cascade foreign key, no cascade found for; " + std::to_string(bottle.vol_0) + "; ";

        if (!error.empty()) {
            OccurredErrorRepository::instance->logError({
                                                                .errCode = "E102",
                                                                .interface = hardwareInterface::runtime,
                                                                .location = __PRETTY_FUNCTION__,
                                                                .data = error,
                                                        });
            ret_error = true;
        }
    }

    return !ret_error;
}

void InitialSetupDataCache::resetDataCache() {
    InitialSetupDataCache::setupData = {};
}
