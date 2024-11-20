//
// Created by jriessner on 14.08.23.
//

#include "utilities/control/Control.h"

int Control::registerInterface(int power) {

    struct interface interface{};
    interface.id = this->currentInterfaceIndex;
    interface.power = power;
    this->interfaces.insert({interface.id, interface});

    this->currentInterfaceIndex += 1;

    return interface.id;
}

int Control::send(int interface, core::control::action cmd, const std::string& data) {
    if (interface != this->activeInterface) {
        return -1;
    }
    struct command exec;
    exec.id = (int) this->commandCounter;
    exec.action = cmd;
    exec.interface = interface;
    exec.input = data;

    this->commands.insert({exec.id, exec});
    this->commandCounter += 1;

    // Keep track of all the stashed commands, and erase commands if parent interface submits another request
    if (this->interfaceCallbackRel.count(interface)) {
        this->commands.erase(this->interfaceCallbackRel[interface]);
    }
    this->interfaceCallbackRel[interface] = exec.id;

    return exec.id;
}

bool Control::claimControl(int inter) {

    if (this->activeInterface == inter) {
        return true;
    }

    if (this->locked) {
        return false;
    }

    if (this->activeInterface == -1) {
        this->activeInterface = inter;

        return true;
    }

    int currentInterfacePower = this->interfaces[this->activeInterface].power;
    struct interface newInterface = this->interfaces[inter];

    if (newInterface.power > currentInterfacePower) {
        this->activeInterface = inter;

        return true;
    }

    return false;
}

void Control::releaseControl(int interface) {
    if(this->activeInterface == interface) {
        this->unlock(interface);
        this->activeInterface = -1;
    }
}

struct Control::command Control::check(uint id) {
    return this->commands[id];
}

std::tuple<bool, struct Control::command> Control::receive() {
    if ( (this->commandReceivedCounter == this->commandCounter)
    || ( !this->commands.count(this->commandReceivedCounter)) ) {

        return {false, {}};
    }
    Control::command ret = this->commands[this->commandReceivedCounter];
    this->commandReceivedCounter += 1;

    return {true, ret};
}

void Control::update(struct Control::command cmd) {
    this->commands[cmd.id] = cmd;
}

bool Control::lock(int interface) {
    if (this->locked) {
        return false;
    }
    if (this->activeInterface == interface) {
        this->locked = true;

        return true;
    }
    return false;
}

bool Control::unlock(int interface) {
    if (this->locked
    && (this->activeInterface == interface)) {

        this->locked = false;

        return true;
    }
    return false;
}
