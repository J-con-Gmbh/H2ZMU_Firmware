//
// Created by jriessner on 01.07.2022.
//

#ifndef H2ZMU_2_BUTTON_H
#define H2ZMU_2_BUTTON_H


#include <map>
#include "data/db/repositories/ParamRepository.h"
#include "conf/Config.h"
#include "interface/modules/dio16/AXL_SE_DI16.h"

enum button {
    MEASURE_START = 1,
    MEASURE_STOP = 2,
    SERVICE_SWITCH = 3,
    CALIBRATION_SWITCH = 4,
    BUTTON_RIGHT = 5,
    BUTTON_LEFT = 6,
    BUTTON_UP = 7,
    BUTTON_DOWN = 8,
    BUTTON_ESCAPE = 9,
    BUTTON_ENTER = 10,
};


class Button {
    std::map<enum button, int> buttonPinMapping;
    std::map<int, bool> wasPressed;
    std::map<int, bool> currentlyPressed;


public:
    static std::shared_ptr<Button> instance;
    std::unique_ptr<AXL_SE_DI16> axcl_dio;
    //void setup(const std::shared_ptr<ParamRepository>& paramRepository);
    void setup();

    bool getStatus(int button);
    bool getStatusNoReset(int button);
    std::vector<enum button> getPressed();
    bool isAnyButtonPressed();

    void updateStatus();

};


#endif //H2ZMU_2_BUTTON_H
