//
// Created by jriessner on 30.06.23.
//

#include "process_logic/sensordata/sensordata_trend.h"

#include <iostream>
#include <cmath>
#include <utility>
#include <list>
#include <algorithm>

#include "structs.h"


void sensordata::Trend::addData(struct dataset set) {
    this->data[set.x] = set.y;
}

void sensordata::Trend::addData(const std::vector<struct dataset>& sets) {
    for (const auto &item: sets) {
        this->addData(item);
    }
}

std::vector<struct dataset> sensordata::Trend::getData(u_int secondsBack) {
    std::vector<struct dataset> ret;
    if (this->data.empty() || secondsBack == 0) {
        return ret;
    }

    auto it = this->data.rbegin();
    long until = it->first - secondsBack;

    for (; it != this->data.rend(); ++it) {
        if (it->first < until) {
            break;
        }
        ret.emplace_back(dataset{.x=it->first, .y=it->second});
    }
    std::reverse(ret.begin(), ret.end());

    return ret;
}

std::vector<struct dataset> sensordata::Trend::getData() {
    return this->getData(0);
}

sensordata::data_trend sensordata::Trend::getTrend(u_int secondsBack) {
    std::vector<dataset> values = this->getData(secondsBack);
    if (values.size() > secondsBack + 1) {
        std::cout << "";
    }

    auto trend = Trend::getTrendForData(values, this->defaultIntervalSec);

    if (trend.inspectedData.size() > secondsBack + 1) {
        std::cout << "";
    }

    this->lastTrend = trend;
//    this->lastTrend = Trend::getTrendForData(values, this->defaultIntervalSec);

    return this->lastTrend;
}

sensordata::data_trend sensordata::Trend::getTrend() {
    return this->getTrend(this->defaultSecondsBack);
}


sensordata::data_trend sensordata::Trend::getTrendForData(std::vector<dataset> values, int intervalSec) {

    size_t data_count = values.size();
    if (data_count == 0) {
        return {};
    }
    size_t data_index_last = data_count - 1;

    enum sensordata::trend lastTrend;
    struct sensordata::data_area lastArea;
    struct sensordata::data_trend dataTrend;// = {.inspectedData = values};

    long lastTs = 0;

    for (int i = (int) data_index_last; i >= 0; --i) {
        dataset current = values[i];
        if (lastTs == 0) {
            lastArea.to = current;
            lastTs = current.x - intervalSec;
        }
        if (current.x <= lastTs) {

            lastArea.from = current;

            if ((lastArea.from.y - lastArea.to.y) > (dataTrend.tolerance * 1000)) {
                lastArea.trend = trend::jump_down;
            } else if ((lastArea.to.y - lastArea.from.y) > (dataTrend.tolerance * 1000)) {
                lastArea.trend = trend::jump_up;
            } else if (isGreaterThan(lastArea.from.y, lastArea.to.y, 0.05)) {
                lastArea.trend = trend::falling;
            } else if (isLessThan(lastArea.from.y, lastArea.to.y, 0.05)) {
                lastArea.trend = trend::rising;
            }
            lastArea.tolerance = dataTrend.tolerance;

            if (lastArea.trend == trend::rising || lastArea.trend == trend::jump_up) {
                dataTrend.strictlyFalling = false;
            } else if (lastArea.trend == trend::falling || lastArea.trend == trend::jump_down) {
                dataTrend.strictlyRising = false;
            }

            dataTrend.areas.emplace_back(lastArea);


            // new cycle
            lastArea = {};
            lastArea.to = current;

            lastTs = current.x - intervalSec;
        }
    }
    std::reverse(dataTrend.areas.begin(), dataTrend.areas.end());

    dataTrend.inspectedData = values;

    float firstValue = values.back().y;
    float lastValue = values.front().y;
    long firstX = values.front().x;
    long lastX = values.back().x;
    if (std::abs(firstValue - lastValue) > (dataTrend.tolerance / 2)) {
        dataTrend.trend = static_cast<trend>(((firstValue - lastValue) > 0) ? -1 : 1);
    }

    dataTrend.pitch_abs = ( (lastValue - firstValue) / ( (float) (lastX - firstX) ) );
    if ( !dataTrend.areas.empty())
        dataTrend.pitch_rel = ( (lastValue - firstValue) / ( (float) dataTrend.areas.size() ) );

    dataTrend.valid = true;

    return dataTrend;
}

/*
 * static functions
 */

sensordata::data_trend sensordata::Trend::getTrendForSensor(Sensor &sensor) {
    std::vector<dataset> last_values = sensor.getLastValuesTime();

    return Trend::getTrendForData(last_values);
}

bool sensordata::Trend::isGreaterThan(float big, float small, float tolerance) {
    return ((big - small) > tolerance);
}

bool sensordata::Trend::isLessThan(float small, float big, float tolerance) {
    return isGreaterThan(big, small, tolerance);
}

std::string sensordata::Trend::toString(const Trend& trend) {
    u_int dataSize = trend.data.size();
    struct sensordata::data_trend dataTrend = trend.lastTrend;
    std::stringstream sstream;
    sstream << "Data Trend" << (!trend.ident.empty() ? "(" + trend.ident + ")" : "") << ":\n";
    sstream << "datasets: " << dataTrend.inspectedData.size() << "\n";
    sstream << "\tgradient:\t" << (((dataTrend.strictlyRising || dataTrend.strictlyFalling) && (dataTrend.trend != 0) ) ? "strictly " : "") <<  ((dataTrend.trend == trend::rising) ? "rising (" : ((dataTrend.trend == 0) ? "steady (": "falling (")) << std::to_string(dataTrend.trend)<< ")\n";
    sstream << "\tabs pitch:\t" << dataTrend.pitch_abs << "\n";
    sstream << "\trel pitch:\t" << dataTrend.pitch_rel << "\n";
    sstream << "\tareas:\t" << dataTrend.areas.size() << ": ";
    sstream.precision(4);
    std::stringstream part;
    for (const auto &item: dataTrend.areas) {
        part << getPitch(item) << " ";

        char c = '-';
        if (item.trend == trend::rising)
            c = '/';
        if (item.trend == trend::falling)
            c = '\\';
        if (item.trend == trend::jump_up || item.trend == trend::jump_down )
            c = '|';
        if (item.trend == trend::steady )
            c = '-';
        sstream << c;
    }
    sstream << " " << part.str();

    return sstream.str();
}

sensordata::Trend::Trend(std::string identifier) {
    this->ident = std::move(identifier);
}

sensordata::trend sensordata::Trend::getTrendForDataPoints(float first, float last, float tolerance) {

    if ((first - last) > (100 * tolerance)) {
        return sensordata::trend::jump_down;
    } else if (std::abs((first - last)) > (100 * tolerance)) {
        return sensordata::trend::jump_up;
    } else if ((first - tolerance) > last) {
        return sensordata::trend::falling;
    } else if ((first + tolerance) < last) {
        return sensordata::trend::rising;
    }

    return sensordata::trend::steady;
}

void sensordata::Trend::setDefaultInterval(int seconds) {
    this->defaultIntervalSec = seconds;
}

void sensordata::Trend::setDefaultSecondsBack(int seconds) {
    this->defaultSecondsBack = seconds;
}

float sensordata::Trend::getPitch(float start_x, float end_x, long start_y, long end_y) {
    float pitch = ( (float)(end_x - start_x) / ( (float) (end_y - start_y) ) );

    return pitch;
}

float sensordata::Trend::getPitch(sensordata::data_area area) {
    float pitch = getPitch(area.to.y, area.from.y, area.to.x, area.from.x);

    return pitch;
}
