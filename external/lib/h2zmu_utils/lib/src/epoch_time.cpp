//
// Created by jriessner on 15.06.23.
//

#include "../include/epoch_time.h"

namespace utils {
    namespace epoch_time {

        long getUnixTimestamp() {
            return std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
        }

        long long getUnixTimestampMs() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
        }

    // "%Y-%m-%d %T.%S"
        std::string getTimestamp(std::string format) {

            bool fHasMillis = false;
            if (size_t index = format.find("%S") != std::string::npos) {
                format = format.substr(0, format.length() - index);
                fHasMillis = true;
            }

            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count() % 1000;

            std::string smillis;
            if (millis < 10) {
                smillis = "00" + std::to_string(millis);
            } else if (millis < 100) {
                smillis = "0" + std::to_string(millis);
            } else {
                smillis = std::to_string(millis);
            }

            std::stringstream ts;

            time_t now1 = time(nullptr);
            ts << std::put_time(localtime(&now1), format.c_str());
            if (fHasMillis)
                ts << smillis;

            return ts.str();
        }

        std::string formatTimestamp(time_t unixTs) {
            std::string format = "%Y-%m-%d %T";
            std::stringstream ts;
            ts << std::put_time(localtime(&unixTs), format.c_str());

            return ts.str();
        }

        std::string formatTimestamp(time_t unixTs, std::string format) {
            if (format.empty())
                format = "%Y-%m-%d %T.%S";
            std::stringstream ts;
            ts << std::put_time(localtime(&unixTs), format.c_str());

            return ts.str();
        }
    }
}