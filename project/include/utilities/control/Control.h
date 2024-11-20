//
// Created by jriessner on 14.08.23.
//

#ifndef H2ZMU_V1_CONTROL_H
#define H2ZMU_V1_CONTROL_H

#include <memory>
#include <string>
#include <map>
#include <functional>

#include "utilities/queue/Queues.h"
#include "utilities/control/CoreFunctionality.h"

class Control {
public:
    struct interface {
        int id;
        int power;
    };

    struct command {
        int id = -1;
        core::control::action action;
        int interface = -1;
        std::string input = {};
        std::string output = {};
        bool finished = false;
        bool returnValue = false;
    };

private:
    int activeInterface = -1;
    bool locked = false;
    std::map<int, interface> interfaces;

    int currentInterfaceIndex = 0;
    uint commandCounter = 0;
    uint commandReceivedCounter = 0;

    std::map<int, std::function<bool(std::string)>> functions;
    uint sub;
    std::map<uint, struct command> commands;
    std::map<int, uint> interfaceCallbackRel;

public:
    //static std::shared_ptr<Control> instance;

    bool lock(int interface);
    bool unlock(int interface);

    int registerInterface(int power);
    bool claimControl(int interface);
    void releaseControl(int interface);

    // Sending member
    int send(int interface, core::control::action cmd, const std::string& data = "");
    struct command check(uint id);

    // Receiving member
    std::tuple<bool, struct command> receive();
    void update(struct command cmd);

};


#endif //H2ZMU_V1_CONTROL_H
