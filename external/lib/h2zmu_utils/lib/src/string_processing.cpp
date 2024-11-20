//
// Created by jriessner on 15.06.23.
//

#include "../include/string_processing.h"

namespace utils {
    namespace strings {

        using namespace utils::strings;

        std::string trimBy(const std::string &toTrim, int byChars) {
            if (toTrim.empty())
                return toTrim;
            return toTrim.substr(0, toTrim.length() - byChars);
        }

        bool isLastOf(const std::string& whole, const std::string& last) {
            int lastLen = (int) last.length();
            int wholeLen = (int) whole.length();

            return ((whole.length() >= lastLen) && (whole.substr(wholeLen - lastLen, lastLen) == last));
        }

        inline std::list<std::string> splitString(const std::string &string, const std::string &delim) {
            std::list<std::string> list;
            std::string locString = string;
            if (string.find(delim) == std::string::npos) {
                list.emplace_back(string);
                return list;
            }
            size_t pos = 0;
            std::string token;
            while ((pos = locString.find(delim)) != std::string::npos ||
                   (pos = locString.find(';')) != std::string::npos) {

                token = locString.substr(0, pos);
                list.emplace_back(token);

                locString.erase(0, pos + 1);

                if ((locString.find(delim) == std::string::npos) && (locString.length() > 0)) {
                    list.emplace_back(locString);
                    break;
                }

            }

            return list;
        }

        std::string getValueFromQuery(const std::string &queryResult, const std::string &key, const std::string &delim) {

            std::list<std::string> valuePairs = splitString(queryResult, delim);

            for (const auto &item: valuePairs) {
                unsigned long p1 = item.find('=');
                std::string currentKey = item.substr(0, p1);
                if (currentKey == key) {
                    std::string s = item.substr(p1 + 1);
                    if (s.find("LEER") != std::string::npos)
                        return SQL_NUMERIC_NULL;

                    return s;
                }
            }

            return SQL_NUMERIC_NULL;
        }


        std::string trim(const std::string &toTrim) {
            std::string string = toTrim;
            while (isspace(toTrim.at(0))) {
                string = string.substr(1, string.length() - 1);
            }
            while (isspace(string.at(string.length() - 1)) || string.at(string.length() - 1) == '\r' ||
                   string.at(string.length() - 1) == '\n') {
                string = string.substr(0, string.length() - 1);
            }

            return string;
        }

        bool findStringIC(const std::string &strHaystack, const std::string &strNeedle) {
            auto it = std::search(
                    strHaystack.begin(), strHaystack.end(),
                    strNeedle.begin(), strNeedle.end(),
                    [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
            );
            return (it != strHaystack.end());
        }

        std::string standardiseDirectoryPath(std::string path) {
            if (!isLastOf(path, "/")) {
                path = path + "/";
            }

            return path;
        }

        std::string replaceAll(std::string str, const std::string& toReplace, const std::string& replacement)  {

            size_t pos = 0;
            while (pos += replacement.length())
            {
                pos = str.find(toReplace, pos);
                if (pos == std::string::npos) {
                    break;
                }

                str.replace(pos, toReplace.length(), replacement);
            }
            return str;
        }

    }
}