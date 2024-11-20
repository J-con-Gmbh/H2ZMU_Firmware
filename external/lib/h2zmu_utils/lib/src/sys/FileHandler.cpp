//
// Created by jriessner on 28.02.2022.
//

#include "../../include/sys/FileHandler.h"
#include "../../include/string_processing.h"
#include "../../include/sys/dump_stacktrace.h"

#include <dirent.h>
#include <iostream>
#include <memory>
#include <sstream>


bool FileHandler::getFileByName(const std::string& path, const std::string& name, std::fstream &file, std::ios_base::openmode policy) {
    return getFileByAbsPath(path + name, file, policy);
}

bool FileHandler::getFileByAbsPath(const std::string &path, std::fstream &file, std::ios_base::openmode policy) {
    file.open(path, policy);
    if (!file) {
        return false;
    }
    return true;
}

std::vector<struct file> FileHandler::getFileNamesFromDirAbsPath(std::string& absolutePath) {

    absolutePath = utils::strings::standardiseDirectoryPath(absolutePath);

    std::vector<struct file> filenames = std::vector<struct file>();

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (absolutePath.c_str())) != nullptr) {
        /* print all the filenames and directories within directory */
        while ((ent = readdir (dir)) != nullptr) {
            if (
                    std::string(ent->d_name) == "."
                    || std::string(ent->d_name) == ".."
                    ) {
                continue;
            }
            struct file f;
            f.name = ent->d_name;
            f.path = absolutePath;
            filenames.emplace_back(f);
        }
        closedir(dir);
    } else {
        std::cerr << "FileHandler: Failed to open directory!\n\t" << absolutePath << std::endl;
    }
    std::sort(filenames.begin(), filenames.end(), [](const file& a, const file& b){
        return (a.name < b.name);
    });
    return filenames;
}

bool FileHandler::saveFile(const std::string& path, const std::string& name, const std::string& content) {

    return saveFile(utils::strings::standardiseDirectoryPath(path) + name, content);
}

bool FileHandler::truncateFile(std::ofstream& file) {

    if (!file)
        return false;
    file << "\n";
    if (file.bad())
        return false;

    return true;
}

bool FileHandler::truncateFileByPath(const std::string& path) {

    std::ofstream file;
    //file.open(path);
    file.open(path, std::fstream::in | std::fstream::out | std::fstream::trunc);

    return FileHandler::truncateFile(file);
}

std::string FileHandler::getFileContent(const std::string& path) {
    std::ifstream file;
    file.open(path);
    if (!file) {
        std::cerr << "FileHandler: Failed to open file!\n\t" << path << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    return content;
}

bool FileHandler::saveFile(const std::string &file, const std::string &content) {
    std::ofstream fstream;
    fstream.open(file);
    if (!fstream) return false;
    fstream << content;
    if (fstream.bad()) return false;
    fstream.close();

    return true;
}
