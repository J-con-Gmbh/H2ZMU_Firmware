/*
 * Filename: h2zmu-axcf/project/include/utilities/tools/configParser.h
 * Created Date: Monday, July 29th 2024, 11:11:58 am
 * Author: PaulEmeriau
 */

#ifndef CONFIGPARSER_H
    #define CONFIGPARSER_H

    #include <iostream>
    #include <map>
    #include <vector>
    #include <fstream>
    #include <string>

// * Function definition

bool isDelim(char c, std::string delim);
std::vector<std::string> strToWordArray(std::string str,
    std::string delim);
std::string getFileContent(std::string pathToFile);
std::map<std::string, std::string> setKeyValues(std::vector<std::string> lines);
std::map<std::string, std::string> confFileParser(std::string path);

#endif