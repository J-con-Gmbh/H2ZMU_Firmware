/*
 * Filename: configParser.cpp
 * Created Date: Monday, July 29th 2024, 10:13:22 am
 * Author: PaulEmeriau
 * Tools > parse config files that are typed like key = value;
 */

#include "utilities/tools/configParser.h"

// ! string to word array
bool isDelim(char c, std::string delim)
{
    for (auto it = delim.begin(); it != delim.end(); it++) {
        if (*it == c)
            return true;
    }
    return false;
}

std::vector<std::string> strToWordArray(std::string str,
    std::string delim)
{
    std::vector<std::string> strArray;
    std::string wordBuffer;

    for (auto it = str.begin(); it != str.end(); it++) {
        if (isDelim(*it, delim) && !wordBuffer.empty()) {
            strArray.push_back(wordBuffer);
            wordBuffer.clear();
        } else {
            wordBuffer += *it;
        }
    }
    if (!wordBuffer.empty())
        strArray.push_back(wordBuffer);
    return strArray;
}

// ! file content
std::string getFileContent(std::string pathToFile)
{
    std::string content;
    std::ifstream ifs(pathToFile);

    if (ifs.fail()) {
        std::cout << "ERROR: " << pathToFile << " no such file or directory" << std::endl;
    }
    content.assign((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());
    return content;
}

// ! get a map of key = value
std::map<std::string, std::string> setKeyValues(std::vector<std::string> lines)
{
    std::vector<std::string> splitLine;
    std::map<std::string, std::string> keyValues;

    for (auto &line : lines) {
        splitLine = strToWordArray(line, "=");
        if (splitLine.size() == 2) {
            keyValues.insert(std::make_pair(splitLine[0], splitLine[1]));
        } else
            std::cout << "ERROR READING CONF FILE" << std::endl;
    }
    return keyValues;
}

std::map<std::string, std::string> confFileParser(std::string path)
{
    std::string conf = getFileContent(path);
    std::vector<std::string> lines = strToWordArray(conf, "\n");
    
    return setKeyValues(lines);
}