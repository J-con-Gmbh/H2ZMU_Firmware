//
// Created by jriessner on 28.02.2022.
//

#ifndef H2ZMU_FILEHANDLER_H
#define H2ZMU_FILEHANDLER_H

#include <fstream>
#include <map>
#include <memory>
#include <vector>

struct file {
    std::string name;
    std::string path;
};

class FileHandler {

public:

    // TODO implement function for appending to file -> Logs
    // TODO add test cases for functions

    static std::fstream currentFile;

    static bool getFileByName(const std::string&, const std::string&, std::fstream &file, std::ios_base::openmode);
    static bool getFileByAbsPath(const std::string &path, std::fstream &file, std::ios_base::openmode policy);
    static std::vector<struct file> getFileNamesFromDirAbsPath(std::string& absolutePath);

    static std::string getFileContent(const std::string&);
    static bool saveFile(const std::string&, const std::string&, const std::string&);
    static bool saveFile(const std::string& file, const std::string& content);

    static bool truncateFile(std::ofstream &file);
    static bool truncateFileByPath(const std::string &path);
};


#endif //H2ZMU_FILEHANDLER_H
