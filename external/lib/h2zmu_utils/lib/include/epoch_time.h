//
// Created by jriessner on 24.05.2022.
//

#ifndef H2ZMU_2_UNIXTIMESTAMP_H
#define H2ZMU_2_UNIXTIMESTAMP_H

#include <chrono>
#include <iomanip>

namespace utils {
    namespace epoch_time {

        long getUnixTimestamp();

        long long getUnixTimestampMs();

        // "%Y-%m-%d %T.%S"
        std::string getTimestamp(std::string format);

        std::string formatTimestamp(time_t unixTs);

        std::string formatTimestamp(time_t unixTs, std::string format);

    }
}
#endif //H2ZMU_2_UNIXTIMESTAMP_H
