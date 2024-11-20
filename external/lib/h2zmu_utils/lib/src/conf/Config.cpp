//
// Created by jriessner on 09.04.2022.
//

#include <iostream>
#include <utility>
#include <regex>

#include "../../include/conf/Config.h"
#include "../../include/sys/FileHandler.h"
#include "../../include/string_processing.h"

namespace utils {

    bool Config::loadConfigFile(const std::string &pathToConf, bool edit) {
        if (this->loaded) {
            // TODO Log Eintrag und Error
            std::cerr << "Config already loaded!" << std::endl;
            return false;
        }
        if (this->onlyMemory) {
            // TODO Log Eintrag und Error
            std::cerr << "Config only in memory!" << std::endl;
            return false;
        }
        this->editable = edit;
        this->confFilePath = pathToConf;

        if (this->onlyOut) {
            return true;
        }

        try {

            bool success = FileHandler::getFileByAbsPath(this->confFilePath, this->configFile, std::ios_base::in);
            if (!success) {
                std::cout << "Datei laden Fehlgeschlagen!" << std::endl << this->confFilePath << std::endl;
                return false;
            }

            std::stringstream buffer;
            buffer << this->configFile.rdbuf();

            this->editable = edit;
            this->configFileContent = buffer.str();
            this->configValues = parseConf(this->configFileContent);

            std::string localConfFileName = this->confFilePath.substr(this->confFilePath.find_last_of('/') + 1);
            if (localConfFileName == "config.txt") {
                std::string localConfFilePath = this->confFilePath.substr(0, this->confFilePath.find_last_of('/'));
                localConfFilePath += "/config.local.txt";
                std::fstream localConfFile;
                if (FileHandler::getFileByAbsPath(localConfFilePath, localConfFile, std::ios_base::in)) {
                    std::stringstream localBuffer;
                    localBuffer << localConfFile.rdbuf();
                    auto localConf = this->parseConf(localBuffer.str());
                    for (const auto &item: localConf) {
                        this->configValues[item.first] = item.second;
                    }
                }

            }

            for (const auto &item: this->configValues) {
                std::string key = item.first;
                std::string value = item.second;
                value = this->replaceToken(value);
                this->configValues[item.first] = value;
            }

            this->loaded = true;

            return true;

        } catch (std::exception &e) {
            // TODO Error und Log Entry
            std::cout << e.what() << std::endl;
            return false;
        }
    }

    bool Config::loadConfig(const std::string &content, bool edit) {
        try {
            this->editable = edit;
            this->configFileContent = content;
            this->configValues = parseConf(this->configFileContent);

            this->loaded = true;

            return true;

        } catch (std::exception &e) {
            /// TODO Error implementieren
        }
        return false;
    }

    std::map<std::string, std::string> Config::parseConf(const std::string &content) {

        std::map<std::string, std::string> valuePairs;

        std::istringstream str(content);
        std::string tmp_line;

        while (std::getline(str, tmp_line)) {
            std::istringstream is_line(tmp_line);
            if (!is_line.str().size()
                || is_line.str().at(0) == '#') {
                continue;
            }
            std::string key;
            if (!std::getline(is_line, key, '=')) {
                continue;
            }
            std::string value;
            if (!std::getline(is_line, value)) {
                continue;
            }
            value = utils::strings::trim(value);
            std::string line_str = is_line.str();
            line_str = line_str.substr(0, line_str.find('='));
            valuePairs.insert(
                    std::pair<std::string, std::string>(line_str, value)
            );
        }

        return valuePairs;
    }

    std::string Config::getValue(const std::string &key) {
        if (this->configValues.count(key)) {
            return this->configValues.find(key)->second;
        }
        return "";
    }

    std::string Config::dumpConfig(const std::string& delim) {
        std::stringstream dump;

        for (const auto &item: this->comments)
            dump << "# " << item.second << "\n";

        for (const auto &item: configValues)
            dump << item.first << delim << item.second << "\n";

        return dump.str();
    }

    std::vector<config::Token> Config::getToken(const std::string &value) {
        std::string string = value;
        std::vector<config::Token> token;
        if (string.find('{') == std::string::npos || string.find('}') == std::string::npos)
            return token;
        int indexTotal = 0;
        while (string.find('{') != std::string::npos) {
            int start = string.find('{');
            int end = string.find('}');

            std::string substr = string.substr(start + 1, end - 1);

            token.push_back({.indexStart=indexTotal + start, .indexEnd=indexTotal + end, .name=substr});
            string = string.substr(end + 1, string.length() - end);
            indexTotal += end + 1;
        }

        return token;
    }

    bool Config::isTrue(const std::string &key) {
        return (getValue(key) == "TRUE");
    }

    bool Config::persistConfig() {
        if (!this->editable) {
            std::cerr << "Config not editable!" << std::endl;
            return false;
        }
        if (this->confFilePath.empty()) {
            std::cerr << "Config path not given!" << std::endl;
            return false;
        }

        try {
            std::stringstream sstream;

            FileHandler::truncateFileByPath(this->confFilePath);
            FileHandler::saveFile(this->confFilePath, this->dumpConfig());

            return true;
        } catch (std::exception &e) {
            return false;
        }
    }

    void Config::setValue(const std::string& key, const std::string& value) {
        if (this->editable) {
            this->configValues[key] = value;
        }
    }

    Config::~Config() {
        this->configFile.close();
    }

    std::string Config::replaceToken(const std::string &value) {
        if (value.find('{') == std::string::npos || value.find('}') == std::string::npos)
            return value;

        size_t start = value.find('{');
        size_t end = value.find('}');
        std::string substr = value.substr(start + 1, end - 1);
        if (!this->configValues.count(substr)) {
            return value;
        }
        std::string newValue;
        newValue = std::regex_replace(value,
                           std::regex("\\{" + substr + "\\}"),
                           this->configValues[substr]);

        if ((newValue.find('{') != std::string::npos) && (newValue.find('}') != std::string::npos)) {
            return this->replaceToken(newValue);
        }
        return newValue;
    }

    void Config::setOnlyOutput(bool out) {
        this->onlyOut = out;
        this->editable = out;
    }
    void Config::setOnlyMemory(bool mem) {
        this->onlyMemory = mem;
    }

    void Config::setPathToConf(std::string path) {
        this->confFilePath = std::move(path);
    }

    void Config::setComment(int pos, std::string value) {
        this->comments[pos] = std::move(value);
    }

    bool Config::hasValue(const std::string &key) {
        return ( !this->getValue(key).empty() );
    }

}