//
// Created by jriessner on 09.04.2022.
//

#ifndef H2ZMU_CONFIG_H
#define H2ZMU_CONFIG_H

#define CONF_DB_FILE "DB_FILE"
#define CONF_DB_CACHING "JUST_CACHING"


#include <fstream>
#include <map>
#include <vector>

namespace utils {

    namespace config {
        struct Token {
            int indexStart;
            int indexEnd;
            std::string name;
        };
    }

    class Config {
        bool loaded = false;
        bool editable = false;
        bool onlyOut = false;
        bool onlyMemory = false;
        std::string confFilePath = {};
        std::fstream configFile;
        std::string configFileContent;
        std::map<int, std::string> comments;
        std::map<std::string, std::string> configValues;
        std::map<std::string, std::string> parseConf(const std::string& content);
        std::string replaceToken(const std::string &value);
    public:
        ~Config();
        void setOnlyOutput(bool out);
        void setOnlyMemory(bool mem);
        void setPathToConf(std::string path);
        bool loadConfigFile(const std::string& pathToConf, bool edit = false);
        bool loadConfig(const std::string& content, bool edit = false);
        std::string getValue(const std::string& key);
        bool hasValue(const std::string& key);
        bool isTrue(const std::string& key);
        static std::vector<config::Token> getToken(const std::string& value);
        bool persistConfig();
        void setValue(const std::string& key, const std::string& value);
        void setComment(int pos, std::string value);
        std::string dumpConfig(const std::string& delim = "=");
    };

}

#endif //H2ZMU_CONFIG_H
