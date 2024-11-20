//
// Created by jriessner on 04.07.23.
//

#ifndef UTILS_CSV_LOADER_H
#define UTILS_CSV_LOADER_H

#include <vector>
#include <string>
#include <cstdlib>
#include <map>

#include "../sys/FileHandler.h"
#include "../structs.h"

namespace data {
namespace csv {

    template <typename T>
    std::vector<struct dataset> loadColumnFromFile(const std::string& pathToFile, u_int rowNr, bool firstRowHeader = true);

    template <typename T>
    std::map<int, std::vector<struct dataset>> loadColumnsFromFile(const std::string& pathToFile, const std::vector<u_int>& rowNrs) {
        return {};
    }

}
}

#endif //UTILS_CSV_LOADER_H
