//
// Created by jriessner on 14.06.23.
//

#ifndef UTILS_RGFH2_H
#define UTILS_RGFH2_H

#include "RgfCorr.h"

enum coefficient {
    b1 = 0,
    b2 = 1,
    b3 = 2,
    b4 = 3,
    b5 = 4,
    c1 = 5,
    c2 = 6,
    c3 = 7,
    c4 = 8,
    c5 = 9,
};

class RgfH2: public RgfCorr {

    /*
    Default values:

    float b1 = -8.30756E-05;
    float b2 = 0.406082149;
    float b3 = -1.55851E-07;
    float b4 = -75.25431103;
    float b5 = 5338.826845;
    float c1 = 5.03296E-06;
    float c2 = -0.002609038;
    float c3 = -3.58268E-09;
    float c4 = 0.57287507;
    float c5 = -40.44092368;
*/

    float b1{};
    float b2{};
    float b3{};
    float b4{};
    float b5{};
    float c1{};
    float c2{};
    float c3{};
    float c4{};
    float c5{};

    void setDefaults();
public:

    RgfH2();
    float zh(float bar, float kelvin) override;

    /**
     * Sets value for specified variable ( enum coefficient ).<br>
     * Defaults:<br>
     * <ul>
     * <li>b1 = -8.30756E-05</li>
     * <li>b2 = 0.406082149</li>
     * <li>b3 = -1.55851E-07</li>
     * <li>b4 = -75.25431103</li>
     * <li>b5 = 5338.826845</li>
     * <li>c1 = 5.03296E-06</li>
     * <li>c2 = -0.002609038</li>
     * <li>c3 = -3.58268E-09</li>
     * <li>c4 = 0.57287507</li>
     * <li>c5 = -40.44092368</li>
     * </ul>
     *
     * @param coefficient
     * @param value
     */
    void setCoefficient(enum coefficient _coefficient, float value);

};


#endif //UTILS_RGFH2_H
