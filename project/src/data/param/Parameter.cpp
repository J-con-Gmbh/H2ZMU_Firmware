//
// Created by jriessner on 22.06.2022.
//

#include "data/param/Parameter.h"

#include <utility>

Parameter::Parameter(int nr, std::string _value): Parameter(nr, param{}) {
    this->value = std::move(_value);
}

Parameter::Parameter(int nr, struct param _param): nr(nr), _param(std::move(_param)) {
    savedAsParam = (_param.id != -1 );
}

int Parameter::getNr() const {
    return this->nr;
}

struct param Parameter::getParam() {
    return _param;
}

bool Parameter::isSavedAsParam() {
    return this->savedAsParam;
}

std::string Parameter::getValue() {
    if (this->isSavedAsParam()) {
        return _param.value;
    } else {
        return this->value;
    }
}

