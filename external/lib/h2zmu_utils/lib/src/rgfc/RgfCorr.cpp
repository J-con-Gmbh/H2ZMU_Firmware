//
// Created by jriessner on 19.05.2022.
//

#include <iostream>
#include <vector>

#include "../../include/rgfc/RgfCorr.h"
#include "../../include/rgfc/RgfH2.h"
#include "../../include/rgfc/RgfN2.h"

std::shared_ptr<RgfCorr> RgfCorr::instance;

RgfCorr::RgfCorr(enum medium _medium, float translationFactor) {
    this->setMedium(_medium);
    this->setTranslationFactor(translationFactor);
}

float RgfCorr::getNormVolume_m3(float volumeLiter, float pressureBar, float temperatureKelvin) {
    float normVolume = volumeLiter * (float) 288.15 / 1 * pressureBar / temperatureKelvin / zh(pressureBar, temperatureKelvin) / 1000;

    return normVolume;
}

float RgfCorr::convertNormVolToKg(float normVolume) const {
    return (normVolume * this->translationFactor_m3_kg);
}

void RgfCorr::setMedium(enum medium medium) {
    this->_medium = medium;
}

void RgfCorr::setTranslationFactor(float factor) {
    this->translationFactor_m3_kg = factor;
}

std::shared_ptr<RgfCorr> RgfCorr::setGlobalSharedPtr(enum medium _medium) {

    switch (_medium) {
        case unset:
            break;
        case H2:
            RgfCorr::instance = std::static_pointer_cast<RgfCorr>(std::make_shared<RgfH2>());
            break;
        case N2:
            RgfCorr::instance = std::static_pointer_cast<RgfCorr>(std::make_shared<RgfN2>());
            break;
    }

    return RgfCorr::instance;
}


