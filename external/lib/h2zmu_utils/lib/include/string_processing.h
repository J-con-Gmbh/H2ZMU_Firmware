//
// Created by jriessner on 12.05.2022.
//

#ifndef H2ZMU_2_STRINGPROCESSING_H
#define H2ZMU_2_STRINGPROCESSING_H

#include <string>
#include <list>
#include <iostream>
#include <algorithm>

#define SQL_NUMERIC_NULL "-1"

namespace utils {
    namespace strings {

        std::string trimBy(const std::string &toTrim, int byChars);

        bool isLastOf(const std::string& whole, const std::string& last);

        std::list<std::string> splitString(const std::string &string, const std::string &delim);

        std::string getValueFromQuery(const std::string &queryResult, const std::string &key, const std::string &delim);

        std::string trim(const std::string &toTrim);

        bool findStringIC(const std::string &strHaystack, const std::string &strNeedle);

        /**
         *
         * Adds slash at end if not already present.
         *
         * @param std::string path to standardise
         * @return std::string standardized path
         */
         std::string standardiseDirectoryPath(std::string path);

         std::string replaceAll(std::string str, const std::string& toReplace, const std::string& replacement);
    }
}

#endif //H2ZMU_2_STRINGPROCESSING_H
