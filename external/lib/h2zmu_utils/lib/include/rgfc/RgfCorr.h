//
// Created by jriessner on 19.05.2022.
//

#ifndef H2ZMU_2_RGFCORR_H
#define H2ZMU_2_RGFCORR_H

#include <memory>

/// Predefine Classes for shared_pointer declaration

enum medium {
    unset,
    H2,
    N2,
};


class RgfCorr {
private:
protected:
    enum medium _medium = medium::unset;
    float translationFactor_m3_kg = -1;
public:
    static std::shared_ptr<RgfCorr> instance;

    RgfCorr(enum medium _medium, float translationFactor);
    void setMedium(enum medium medium);

    /**
     * Sets factor for linear translation between nnm3 and kg<br>
     * Default values:<br>   h2 = 0.08988<br>   n2 = 1.169
     * <br>
     * @param factor
     */
    void setTranslationFactor(float factor);

    virtual float zh(float bar, float kelvin) = 0;
    float convertNormVolToKg(float normVolume) const;
    float getNormVolume_m3(float volumeLiter, float pressureBar, float temperatureKelvin);


    /**
     * Creates shared_ptr for rgf correction
     * @param medium
     * @return shared_ptr\<RgfCorr\> to global object
     */
    static std::shared_ptr<RgfCorr> setGlobalSharedPtr(enum medium _medium);

};

#endif //H2ZMU_2_RGFCORR_H
