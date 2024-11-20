//
// Created by jriessner on 19.06.23.
//

#include "../../include/geom/LinearRegression.h"

namespace utils {
    namespace geom {

        LinearRegression::LinearRegression() {
            coeff = 0;
            constTerm = 0;
            sum_y = 0;
            sum_y_square = 0;
            sum_x_square = 0;
            sum_x = 0;
            sum_xy = 0;
        }

        void LinearRegression::calculateCoefficient() {
            float N = (float) x.size();
            float numerator
                    = (N * sum_xy - sum_x * sum_y);
            float denominator
                    = (N * sum_x_square - sum_x * sum_x);
            coeff = numerator / denominator;
        }

        void LinearRegression::calculateConstantTerm() {
            float N = (float) x.size();
            float numerator
                    = (sum_y * sum_x_square - sum_x * sum_xy);
            float denominator
                    = (N * sum_x_square - sum_x * sum_x);
            constTerm = numerator / denominator;
        }

        int LinearRegression::sizeOfData() {
            return (int) x.size();
        }

        float LinearRegression::coefficient() {
            if (coeff == 0)
                calculateCoefficient();
            return coeff;
        }

        float LinearRegression::constant() {
            if (constTerm == 0)
                calculateConstantTerm();
            return constTerm;
        }

        std::string LinearRegression::PrintBestFittingLine() const {
            std::stringstream str;
            str << "The best fitting line is y = " << coeff << " x + " << constTerm;

            return str.str();
        }

        void LinearRegression::takeInput(const std::vector<struct dataset> &data) {
            for (const auto &item: data) {
                float xi = (float) item.x;
                float yi = item.y;
                sum_xy += xi * yi;
                sum_x += xi;
                sum_y += yi;
                sum_x_square += xi * xi;
                sum_y_square += yi * yi;
                x.push_back(xi);
                y.push_back(yi);
            }
        }

        void LinearRegression::resetInput() {
            x = {};
            y = {};
            coeff = 0;
            constTerm = 0;
            sum_y = 0;
            sum_y_square = 0;
            sum_x_square = 0;
            sum_x = 0;
            sum_xy = 0;
        }

        void LinearRegression::calculateRegression() {
            calculateCoefficient();
            calculateConstantTerm();
        }

        float LinearRegression::predict(long x) const {
            return coeff * (float) x + constTerm;
        }

        float LinearRegression::errorSquare() {
            float ans = 0;
            for (int i = 0;
                 i < x.size(); i++) {
                ans += ((predict(x[i]) - y[i])
                        * (predict(x[i]) - y[i]));
            }
            return ans;
        }

        float LinearRegression::errorIn(float num) {
            for (int i = 0;
                 i < x.size(); i++) {
                if (num == x[i]) {
                    return (y[i] - predict(x[i]));
                }
            }
            return 0;
        }

        float LinearRegression::getCoeff() const {
            return coeff;
        }
    }
}