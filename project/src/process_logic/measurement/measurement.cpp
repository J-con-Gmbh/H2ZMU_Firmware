//
// Created by jriessner on 22.06.23.
//

#include "process_logic/measurement/measurement.h"
#include "data/db/repositories/CascadeRepository.h"
#include "data/db/repositories/CascadestateRepository.h"
#include "data/db/repositories/SensorstateRepository.h"
#include "data/db/repositories/OccurredErrorRepository.h"
#include "data/db/repositories/OccurredWarningRepository.h"
#include "sys/dump_stacktrace.h"

namespace msrmnt {

    /**
     * Create Measurement skeleton and persist to database.<br>
     * In this function no vessel-, cascade- or sensor-states are created.
     *
     * @param userId
     * @param externalId
     * @return
     */
    std::tuple<bool, struct measurement> create_measurement(int userId, const std::string& externalId) {
        struct measurement _measurement;

        _measurement.user_id = userId;
        _measurement.timestamp_start = utils::epoch_time::getUnixTimestamp();
        _measurement.external_measurement_id = externalId;

        bool success = MeasurementRepository::instance->createMeasurement(_measurement);

        return std::make_tuple(success, _measurement);
    }

    /**
     * Records the vesselstate for the measurement-start.<br>
     * In this function creates and persists vessel-, cascade- and sensor-states.
     *
     * @param _measurement
     * @return
     */
    bool start_measurement (struct measurement& _measurement) {

        struct vesselstate vstate_start = Vessel::instance->getVesselState();

        if ( !VesselstateRepository::instance->newVesselState(vstate_start) )  {
            OccurredErrorRepository::instance->logError({
                .errCode = "E001",
                .interface = hardwareInterface::runtime,
                .location = __PRETTY_FUNCTION__,
                .data = "Failed to create vesselState! Stacktrace: " + utils::sys::dump_stacktrace(),
            });

            _measurement.fk_thrown_errors.push_back(OccurredErrorRepository::instance->getCurrentId());

            return false;
        }
 
        _measurement.amountStartNm3 = vstate_start.norm_volume;

        if ( !MeasurementRepository::instance->addVesselStateStart(_measurement, vstate_start) ) {
            OccurredErrorRepository::instance->logError({
                .errCode = "E001",
                .interface = hardwareInterface::runtime,
                .location = __PRETTY_FUNCTION__,
                .data = "Failed to add vesselState to Measurement! Stacktrace: " + utils::sys::dump_stacktrace(),
            });

            _measurement.fk_thrown_errors.push_back(OccurredErrorRepository::instance->getCurrentId());

            return false;
        }

        return true;
    }

    /**
     * Records the vesselstate for the measurement-end.<br>
     * In this function creates and persists vessel-, cascade- and sensor-states.
     *
     * @param _measurement
     * @return
     */
    bool end_measurement (struct measurement& _measurement) {

        struct vesselstate vstate_end = Vessel::instance->getVesselState();

        if ( !VesselstateRepository::instance->newVesselState(vstate_end) ) {
            OccurredErrorRepository::instance->logError({
                .errCode = "E001",
                .interface = hardwareInterface::runtime,
                .location = __PRETTY_FUNCTION__,
                .data = "Failed to create vesselState! Stacktrace: " + utils::sys::dump_stacktrace(),
            });

            _measurement.fk_thrown_errors.push_back(OccurredErrorRepository::instance->getCurrentId());

            return false;
        }

        _measurement.amountEndNm3 = vstate_end.norm_volume;

        if ( !MeasurementRepository::instance->addVesselStateEnd(_measurement, vstate_end) ) {
            OccurredErrorRepository::instance->logError({
                .errCode = "E001",
                .interface = hardwareInterface::runtime,
                .location = __PRETTY_FUNCTION__,
                .data = "Failed to add vesselState to measurement! Stacktrace: " + utils::sys::dump_stacktrace(),
            });

            _measurement.fk_thrown_errors.push_back(OccurredErrorRepository::instance->getCurrentId());

            return false;
        }

        if ( !persist_measurement(_measurement) ) {
            OccurredErrorRepository::instance->logError({
                .errCode = "E001",
                .interface = hardwareInterface::runtime,
                .location = __PRETTY_FUNCTION__,
                .data = "Failed to persist measurement! Stacktrace: " + utils::sys::dump_stacktrace(),
            });

            _measurement.fk_thrown_errors.push_back(OccurredErrorRepository::instance->getCurrentId());

            return false;
        }

        return true;
    }


    /**
     * Test the measurement-struct for common errors and inconsistencies,<br>
     * and set the measurement valid
     *
     * @param _measurement
     */
    bool validate_measurement(struct measurement& _measurement) {
        // TODO implement validation
        _measurement.valid = true;

        return true;
    }


    /**
     * Adds and persists cascade-current_state for the given measurement-struct and cascade to the measurement start.
     *
     * @param _measurement
     * @param cascadeId
     * @return
     */
    bool set_start_point_for_cascade(struct measurement& _measurement, int cascadeId) {
        return set_point_for_cascade(_measurement.fk_vessel_state_start, cascadeId);
    }

    /**
     * Adds and persists cascade-current_state for the given measurement-struct and cascade to the measurement end.
     *
     * @param _measurement
     * @param cascadeId
     * @return
     */
    bool set_end_point_for_cascade(struct measurement& _measurement, int cascadeId) {
        bool success = set_point_for_cascade(_measurement.fk_vessel_state_end, cascadeId);
        if (!success) {
            return false;
        }
        return success;
    }

    /**
     * Adds and persists cascade-current_state for the given measurement-struct and cascade to the measurement end.
     *
     * @param _measurement
     * @param cascadeId
     * @return
     */
    bool set_point_for_cascade(int fk_vesselstate, int cascadeId) {

        std::unique_ptr<Cascade> cascade = Vessel::instance->getCascadeById(cascadeId);
        if (!cascade) {
            std::stringstream msg;
            msg << "Cascade " << cascadeId << " not found!";

            OccurredErrorRepository::instance->logError({
                .errCode = "E001",
                .interface = hardwareInterface::runtime,
                .location = __PRETTY_FUNCTION__,
                .data = msg.str() + utils::sys::dump_stacktrace(),
            });

            return false;
        }

        struct vesselstate vstate = VesselstateRepository::instance->getVesselstate(fk_vesselstate);

        // Filter already processed cascades
        for (const auto &item: vstate.fk_cascadestates) {
            struct cascadestate tmp = CascadestateRepository::instance->getCascadestateById(item);
            if (tmp.fk_cascade == cascadeId) {
                OccurredErrorRepository::instance->logError({
                    .errCode = "E001",
                    .interface = hardwareInterface::runtime,
                    .location = __PRETTY_FUNCTION__,
                    .data = "Multiple states of same cascade for single vessel current_state! Stacktrace: " + utils::sys::dump_stacktrace(),
                });

                return false;
            }
        }

        struct cascadestate cstate = cascade->getCascadeState();

        if ( !CascadestateRepository::instance->newCascadestate(cstate) ) {
                OccurredErrorRepository::instance->logError({
                    .errCode = "E001",
                    .interface = hardwareInterface::runtime,
                    .location = __PRETTY_FUNCTION__,
                    .data = "Failed to create cascadeState to Measurement! Stacktrace: " + utils::sys::dump_stacktrace(),
                });

            return false;
        }

        vstate.fk_cascadestates.emplace_back(cstate.id);
        vstate.norm_volume += cstate.norm_volume;
        if ( !VesselstateRepository::instance->updateVesselState(vstate) ) {      
            OccurredErrorRepository::instance->logError({
                .errCode = "E001",
                .interface = hardwareInterface::runtime,
                .location = __PRETTY_FUNCTION__,
                .data = "Failed to update vesselState! Stacktrace: " + utils::sys::dump_stacktrace(),
            });

            return false;
        }

        return true;
    }

    bool finalize_measurement(struct measurement& _measurement) {
        struct vesselstate v_state_start = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_start);
        struct vesselstate v_state_end = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_end);

        _measurement.amountStartNm3 = v_state_start.norm_volume;
        _measurement.amountEndNm3 = v_state_end.norm_volume;


        return true;
    }

    bool persist_measurement(measurement &_measurement) {

        _measurement.timestamp_end = utils::epoch_time::getUnixTimestamp();

        if ( !validate_measurement(_measurement) ) {
            OccurredErrorRepository::instance->logError({
                .errCode = "E001",
                .interface = hardwareInterface::runtime,
                .location = __PRETTY_FUNCTION__,
                .data = "Failed to validate measurement! Stacktrace: " + utils::sys::dump_stacktrace(),
            });

            _measurement.fk_thrown_errors.push_back(OccurredErrorRepository::instance->getCurrentId());
        }

        if ( !MeasurementRepository::instance->finalizeMeasurement(_measurement) ) {
            OccurredErrorRepository::instance->logError({
                .errCode = "E001",
                .interface = hardwareInterface::runtime,
                .location = __PRETTY_FUNCTION__,
                .data = "Failed to add vesselState to Measurement! Stacktrace: " + utils::sys::dump_stacktrace(),
            });

            _measurement.fk_thrown_errors.push_back(OccurredErrorRepository::instance->getCurrentId());

            return false;
        }


        return true;
    }

    std::string dump_measurement(const struct measurement& _measurement) {

        struct vesselstate v_state_start = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_start);
        struct vesselstate v_state_end = VesselstateRepository::instance->getVesselstate(_measurement.fk_vessel_state_end);

        std::vector<int> fk_c_state_start = v_state_start.fk_cascadestates;
        std::vector<int> fk_c_state_end = v_state_end.fk_cascadestates;

        std::stringstream sstream;
        sstream << "Messpunkt: " << _measurement.id;
        sstream << "\n\ttime start: " << utils::epoch_time::formatTimestamp(_measurement.timestamp_start);
        sstream << "\n\ttime end:   " << utils::epoch_time::formatTimestamp(_measurement.timestamp_end);
        sstream << "\n\tnm3 start:  " << _measurement.amountStartNm3;
        sstream << "\n\tnm3 end:    " << _measurement.amountEndNm3;
        sstream << "\n\tVesselstate Start: " << v_state_start.id;
        sstream << "\n\t\tid:      " << v_state_start.id;
        sstream << "\n\t\tgeom m3: " << v_state_start.geom_volume_0;
        sstream << "\n\t\tnorm m3: " << v_state_start.norm_volume;
        for (const auto &item: v_state_start.fk_cascadestates) {
            struct cascadestate c_state = CascadestateRepository::instance->getCascadestateById(item);
            sstream << "\n\t\tCascade:      " << c_state.fk_cascade;
            sstream << "\n\t\tcurrent_state id:      " << c_state.id;
            sstream << "\n\t\t\tgeom m3: " << c_state.geom_volume_corr;
            sstream << "\n\t\t\tnorm m3: " << c_state.norm_volume;
            sstream << "\n\t\t\tp bar:   " << SensorstateRepository::instance->getSensorstate(c_state.fk_pressure_upper_sensorstate).value;
            sstream << "\n\t\t\tt °C:    " << SensorstateRepository::instance->getSensorstate(c_state.fk_temperature_values[0]).value;
        }
        sstream << "\n\tVesselstate End: " << v_state_end.id;
        sstream << "\n\t\tid:      " << v_state_end.id;
        sstream << "\n\t\tgeom 0:  " << v_state_end.geom_volume_0;
        sstream << "\n\t\tnorm m3: " << v_state_end.norm_volume;
        for (const auto &item: v_state_end.fk_cascadestates) {
            struct cascadestate c_state = CascadestateRepository::instance->getCascadestateById(item);
            sstream << "\n\t\tCascade:      " << c_state.fk_cascade;
            sstream << "\n\t\tcurrent_state id:      " << c_state.id;
            sstream << "\n\t\t\tgeom p:  " << c_state.geom_volume_corr;
            sstream << "\n\t\t\tnorm m3: " << c_state.norm_volume;
            sstream << "\n\t\t\tp bar:   " << SensorstateRepository::instance->getSensorstate(c_state.fk_pressure_upper_sensorstate).value;
            sstream << "\n\t\t\tt °C:    " << SensorstateRepository::instance->getSensorstate(c_state.fk_temperature_values[0]).value;
        }


        return sstream.str();
    }

}
