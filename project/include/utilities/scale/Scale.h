//
// Created by jriessner on 18.08.23.
//

#ifndef H2ZMU_V1_SCALE_H
#define H2ZMU_V1_SCALE_H

#include <memory>
#include "modbus_interface.h"
#include "queue/Queue.h"

struct scale_state {
    float default_value = -1;
    float gross_value = -1;
    float tare_value = -1;
    float net_value = -1;
    float raw_value = -1;
    float unit = -1;
};


class Scale {
private:

    enum Register {
        DEFAULT = 0,
        GROSS = 8,
        TARE = 10,
        NET = 12,
        UNIT = 14,
        RAW = 16,
    };

    mb::ModbusClient modbusClient;
    bool setup(const std::string& fd, bool debug = false);

public:
    static std::shared_ptr<Scale> instance;

    Scale();
    //utils::Queue<scale_state> queue;

    float fetchData();

};


#endif //H2ZMU_V1_SCALE_H
