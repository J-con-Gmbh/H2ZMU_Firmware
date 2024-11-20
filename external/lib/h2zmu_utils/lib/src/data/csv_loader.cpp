//
// Created by jriessner on 10.07.23.
//

#include "../../include/data/csv_loader.h"

namespace data {
    namespace csv {

        std::vector<struct dataset> loadColumnFromFile(const std::string& pathToFile, u_int rowNr, bool firstRowHeader) {
            std::vector<struct dataset> ret;
            std::string contentCsv = FileHandler::getFileContent(pathToFile);
            if (contentCsv.empty()) {
                return ret;
            }

            return ret;
        }

    }
}