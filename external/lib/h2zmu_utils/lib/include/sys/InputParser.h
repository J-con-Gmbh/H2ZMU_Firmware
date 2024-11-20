//
// Created by jriessner on 21.07.23.
//

#ifndef UTILS_MAIN_INPUTPARSER_H
#define UTILS_MAIN_INPUTPARSER_H

#include <string>
#include <vector>
#include <algorithm>

class InputParser {
private:
    std::vector <std::string> tokens;
public:
    InputParser (int &argc, char **argv);

    const std::string& getCmdOption(const std::string &option) const;

    bool cmdOptionExists(const std::string &option) const;
};


#endif //UTILS_MAIN_INPUTPARSER_H
