//
// Created by jriessner on 14.06.23.
//

#include "../../include/rgfc/RgfH2.h"

/*
float RgfH2::b1;
float RgfH2::b2;
float RgfH2::b3;
float RgfH2::b4;
float RgfH2::b5;
float RgfH2::c1;
float RgfH2::c2;
float RgfH2::c3;
float RgfH2::c4;
float RgfH2::c5;
*/

RgfH2::RgfH2(): RgfCorr(medium::H2, 0.08988) {
    this->setDefaults();
}

float RgfH2::zh(float bar, float kelvin) {
    float b_t = b1 + b2 / kelvin + b3 * kelvin + b4 / (kelvin * kelvin) + b5 / (kelvin * kelvin * kelvin);
    float c_t = c1 + c2 / kelvin + c3 * kelvin + c4 / (kelvin * kelvin) + c5 / (kelvin * kelvin * kelvin);
    float z_t_p = 1 + b_t * bar + c_t * bar * bar;

    return z_t_p;
}

void RgfH2::setDefaults() {
    this->b1 =   -8.30756E-05;
    this->b2 =    0.406082149;
    this->b3 =   -1.55851E-07;
    this->b4 =  -75.25431103;
    this->b5 = 5338.826845;
    this->c1 =    5.03296E-06;
    this->c2 =   -0.002609038;
    this->c3 =   -3.58268E-09;
    this->c4 =    0.57287507;
    this->c5 =  -40.44092368;
}

void RgfH2::setCoefficient(enum coefficient  _coefficient, float value) {
    switch (_coefficient) {
        case ::b1:
            this->b1 = value;
            break;
        case ::b2:
            this->b2 = value;
            break;
        case ::b3:
            this->b3 = value;
            break;
        case ::b4:
            this->b4 = value;
            break;
        case ::b5:
            this->b5 = value;
            break;
        case ::c1:
            this->c1 = value;
            break;
        case ::c2:
            this->c2 = value;
            break;
        case ::c3:
            this->c3 = value;
            break;
        case ::c4:
            this->c4 = value;
            break;
        case ::c5:
            this->c5 = value;
            break;
    }
}
