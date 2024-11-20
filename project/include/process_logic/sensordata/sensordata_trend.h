//
// Created by jriessner on 30.06.23.
//

#ifndef H2ZMU_V1_SENSORDATA_TRENDS_H
#define H2ZMU_V1_SENSORDATA_TRENDS_H


#include <map>
#include "vessel/Sensor.h"

namespace sensordata {

    enum trend {
        jump_down = -2,
        falling = -1,
        steady = 0,
        rising = 1,
        jump_up = 2
    };

    struct data_area {
        enum trend trend = trend::steady;
        float tolerance;
        struct dataset from;
        struct dataset to;
    };

    struct data_trend {
        bool valid = false;
        enum trend trend = trend::steady;
        float tolerance = 0.05; /// TODO Echtdaten analysieren und realsistischen Wert ermitteln
        bool strictlyRising = true;
        bool strictlyFalling = true;
        float pitch_abs = 0; /// Steigung
        float pitch_rel = 0; /// Steigung
        u_int peaks = 0;
        u_int valleys = 0;
        std::vector<struct data_area> areas{};
        std::vector<struct dataset> inspectedData{};
    };

    class Trend {
    private:
        std::string ident;
        int defaultIntervalSec = 20;
        int defaultSecondsBack = 300;
        struct data_trend lastTrend {};
        std::map<long, float, std::less<>> data;
    public:
        explicit Trend(std::string identifier = "");
        void setDefaultInterval(int seconds);
        void setDefaultSecondsBack(int seconds);

        void addData(struct dataset set);
        void addData(const std::vector<struct dataset>& data);
        std::vector<struct dataset> getData();
        std::vector<struct dataset> getData(u_int secondsBack);
        struct data_trend getTrend();
        struct data_trend getTrend(u_int secondsBack);

        static bool isGreaterThan(float big, float small, float tolerance = 0);
        static bool isLessThan(float small, float big, float tolerance = 0);
        static enum trend getTrendForDataPoints(float small, float big, float tolerance = 0);
        static struct data_trend getTrendForSensor(Sensor &sensor);
        //static struct data_trend getTrendForData(std::vector<dataset> values);
        static struct data_trend getTrendForData(std::vector<dataset> values, int intervalSec = 20);

        static std::string toString(const Trend& trend);
        static float getPitch(float start_x, float end_x, long start_y, long end_y);
        static float getPitch(sensordata::data_area area);
    };


}

#endif //H2ZMU_V1_SENSORDATA_TRENDS_H
