//
// Created by jriessner on 25.07.23.
//trend.areas = {std::vector<sensordata::data_area>}

#include "process_logic/measurement/dynamic/dynamic_measurement_alloc.h"

std::tuple<bool, sensordata::data_area> msrmnt::detect_trend_change(const sensordata::data_trend& trend) {
    std::vector<sensordata::data_area> data = trend.areas;
    if (data.empty()) {
        return std::make_tuple(false, sensordata::data_area{});
    }

    sensordata::trend start = data[0].trend;

    int c = 0;
    for (const auto &item: data) {
        if (item.trend != start) {
            return std::make_tuple(true, item);
        }
        c += 1;
    }
    return std::make_tuple(false, sensordata::data_area{});
}

std::tuple<bool, dataset> msrmnt::detect_first_pressure_jump(sensordata::data_trend trend) {

    auto i = trend.areas.rbegin();

    dataset beforeJump{.x = 0, .y = -1};

    for (; i != trend.areas.rend(); ++i) {

        long indexStart = i->from.x;
        long indexEnd = i->to.x;

        for (const auto &item: trend.inspectedData) {

            // Skip if not in specified time range
            if (item.x < indexStart || item.x > indexEnd) {
                continue;
            }

            if ( item.y > (beforeJump.y + (100 * trend.tolerance)) ) {
                return std::make_tuple( true, item);
            }
        }
    }

    return std::make_tuple( false, dataset{});
}

dataset msrmnt::get_peak_dataset_in_area(const sensordata::data_trend& trend, const sensordata::data_area &area) {
    dataset set{0, 0};
    for (const auto &item: trend.inspectedData) {
        if ( (area.from.x < item.x) || (area.to.x > item.x) ) {
            continue;
        }
        if (set.y < item.y || set.x == 0) {
            set = item;
        }
    }

    return set;
}