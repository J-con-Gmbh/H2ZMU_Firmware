//
// Created by jriessner on 14.06.23.
//

#ifndef UTILS_RGFN2_H
#define UTILS_RGFN2_H

#include <vector>

#include "RgfCorr.h"

class RgfN2: public RgfCorr {
private:
    static std::vector<std::vector<float>> rgf_n2;
    static std::vector<int> rgf_n2_t;
    static std::vector<int> rgf_n2_p;

public:

    RgfN2();
    float zh(float bar, float kelvin) override;
};

#endif //UTILS_RGFN2_H
