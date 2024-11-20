//
// Created by jriessner on 01.07.2022.
//

#include <chrono>
#include <thread>

#include "interface/hmi/Button.h"

std::shared_ptr<Button> Button::instance;

void Button::setup() {

    
    buttonPinMapping = {
            {BUTTON_UP,          std::stoi(ParamRepository::instance->getParamByNr(20).value)},
            {BUTTON_DOWN,        std::stoi(ParamRepository::instance->getParamByNr(21).value)},
            {BUTTON_LEFT,        std::stoi(ParamRepository::instance->getParamByNr(22).value)},
            {BUTTON_RIGHT,       std::stoi(ParamRepository::instance->getParamByNr(23).value)},
            {BUTTON_ESCAPE,      std::stoi(ParamRepository::instance->getParamByNr(24).value)},
            {BUTTON_ENTER,       std::stoi(ParamRepository::instance->getParamByNr(25).value)},
            {MEASURE_START,      std::stoi(ParamRepository::instance->getParamByNr(26).value)},
            {MEASURE_STOP,       std::stoi(ParamRepository::instance->getParamByNr(27).value)},
            {SERVICE_SWITCH,     std::stoi(ParamRepository::instance->getParamByNr(28).value)},
            {CALIBRATION_SWITCH, std::stoi(ParamRepository::instance->getParamByNr(29).value)}
    };

    /*
    buttonPinMapping = {{ BUTTON_DOWN,        7 - 1  },
                        { BUTTON_UP,          8 - 1  },
                        { BUTTON_RIGHT,       9 - 1  },
                        { BUTTON_LEFT,        10 - 1 },
                        { BUTTON_ENTER,       11 - 1 },
                        { BUTTON_ESCAPE,      12 - 1 },
                        { MEASURE_STOP,       13 - 1 },
                        { MEASURE_START,      14 - 1 },
                        { SERVICE_SWITCH,     15 - 1 },
                        { CALIBRATION_SWITCH, 16 - 1 }
                       
};
*/
    this->axcl_dio = std::make_unique<AXL_SE_DI16>("Arp.Plc.Eclr/wDI16");

}

bool Button::getStatus(int button) {
    if (!this->wasPressed.count(button)) {
        return false;
    }
    bool wasPrssd = wasPressed[button];
    wasPressed[button] = false;

    return wasPrssd;
}

bool Button::getStatusNoReset(int button) {
    if (!this->wasPressed.count(button)) {
        return false;
    }
    bool wasPrssd = wasPressed[button];

    return wasPrssd;
}

void Button::updateStatus() {

    axcl_dio->update();
    auto state = axcl_dio->getCurrentState();

    for (const auto &item : Button::buttonPinMapping) {
        int read = state[item.second];
        if (read != 0 && !currentlyPressed[item.first]) {
            wasPressed[item.first] = true;
            std::cout << "Name: " << item.first << " Currently Pressed: " << currentlyPressed[item.first] << std::endl;
        }
        currentlyPressed[item.first] = (read == 1);

    }
}

bool Button::isAnyButtonPressed() {

    std::vector<enum button> ret;
    for (const auto &item : buttonPinMapping) {
        if (getStatusNoReset(item.first)){
            return true;
        }
    }

    return false;
}

std::vector<enum button> Button::getPressed() {
    std::vector<enum button> ret;
    for (const auto &item : buttonPinMapping) {
        if (getStatus(item.first)){
            ret.push_back((enum button) item.first);
        }
    }

    return ret;
}
