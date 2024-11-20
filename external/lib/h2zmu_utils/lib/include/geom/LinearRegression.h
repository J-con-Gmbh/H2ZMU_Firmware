//
// Created by jriessner on 19.06.23.
//

#ifndef UTILS_LINEARREGRESSION_H
#define UTILS_LINEARREGRESSION_H

#include <vector>
#include <iostream>
#include <sstream>

namespace utils {
    namespace geom {

        struct dataset {
            long x;
            float y;
        };


        class LinearRegression {

            // Dynamic array which is going
            // to contain all (i-th x)
            std::vector<float> x;

            // Dynamic array which is going
            // to contain all (i-th y)
            std::vector<float> y;

            // Store the coefficient/slope in
            // the best fitting line
            float coeff;

            // Store the constant term in
            // the best fitting line
            float constTerm;

            // Contains sum of product of
            // all (i-th x) and (i-th y)
            float sum_xy;

            // Contains sum of all (i-th x)
            float sum_x;

            // Contains sum of all (i-th y)
            float sum_y;

            // Contains sum of square of
            // all (i-th x)
            float sum_x_square;

            // Contains sum of square of
            // all (i-th y)
            float sum_y_square;

        public:
            // Constructor to provide the default
            // values to all the terms in the
            // object of class LinearRegression
            LinearRegression();

            // Function that calculate the coefficient/
            // slope of the best fitting line
            void calculateCoefficient();

            // Member function that will calculate
            // the constant term of the best
            // fitting line
            void calculateConstantTerm();

            // Function that return the number
            // of entries (xi, yi) in the data set
            int sizeOfData();

            // Function that return the coefficient/
            // slope of the best fitting line
            float coefficient();

            // Function that return the constant
            // term of the best fitting line
            float constant();

            // Function that print the best
            // fitting line
            std::string PrintBestFittingLine() const;

            // Function to take input from the dataset
            void takeInput(const std::vector<struct dataset> &data);

            void resetInput();

            void calculateRegression();

            // Function to predict the value
            // corresponding to some input
            float predict(long x) const;

            // Function that returns overall
            // sum of square of errors
            float errorSquare();

            // Functions that return the error
            // i.e the difference between the
            // actual value and value predicted
            // by our model
            float errorIn(float num);

            float getCoeff() const;
        };
    }
}

#endif //UTILS_LINEARREGRESSION_H
