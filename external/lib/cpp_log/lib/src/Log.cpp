//
// Created by jriessner on 12.05.2022.
//

#include <thread>
#include <iostream>
#include <vector>
#include "../include/Log.h"

using namespace h2zmu;

Log Log::instance;
bool Log::verbose = false;
struct log_config Log::config = {};

loglevel Log::logstate = loglevel::debug;

Log::Log() = default;

void Log::open(const std::string& logDir){
    std::string mainLog = logDir + "/h2zmu.log";
    std::string errLog = logDir + "/error.log";
    std::string funCallLog = logDir + "/function.log";
    std::string sqlLog = logDir + "/sql.log";
    std::string accessLog = logDir + "/access.log";
    std::string sensorLog = logDir + "/sensor.log";

    std::string touchFilesCmd = "touch " + mainLog + " " + errLog + " " + funCallLog + " " + sqlLog + " " + accessLog + " " + sensorLog;

    std::string mkdirCmd = "mkdir -p " + logDir;
    system(mkdirCmd.c_str());
    system(touchFilesCmd.c_str());

    instance.allLog.open(mainLog, std::fstream::out | std::fstream::app);
    instance.errorLog.open(errLog, std::ios::out | std::fstream::app);
    instance.functionLog.open(funCallLog, std::ios::out | std::fstream::app);
    instance.sqlLog.open(sqlLog, std::ios::out | std::fstream::app);
    instance.accessLog.open(accessLog, std::ios::out | std::fstream::app);
    instance.sensorLog.open(sensorLog, std::ios::out | std::fstream::app);
}

void Log::close() {
    instance.allLog.close();
    instance.errorLog.close();
    instance.functionLog.close();
    instance.sqlLog.close();
    instance.accessLog.close();
    instance.sensorLog.close();
}

void Log::write(const struct logentry& logentry) {
    write(logentry, logfile::info);
}

void Log::logfunc(const struct logentry& logentry) {
    write(logentry, logfile::func);
}

void Log::error(struct logentry logentry) {
    write(logentry, logfile::error);
}

void Log::write(const struct logentry& logentry, int _logfile) {

    if (logstate > logentry.loglvl) {
        return;
    }

    std::string message = messageFromLogEntry(logentry, _logfile);

    switch (_logfile) {
        case logfile::error:
            instance.errorLog.is_open() ? instance.errorLog << message << std::flush : std::cout << message << std::flush;
            break;
        case logfile::func:
            instance.functionLog.is_open() ? instance.functionLog << message << std::flush : std::cout << message << std::flush;
            break;
        case logfile::sql:
            instance.sqlLog.is_open() ? instance.sqlLog << message << std::flush : std::cout << message << std::flush;
            break;
        case logfile::log_access:
            instance.accessLog.is_open() ? instance.accessLog << message << std::flush : std::cout << message << std::flush;
            break;
        case logfile::info:
            instance.allLog.is_open() ? instance.allLog << message << std::flush : std::cout << message << std::flush;
            break;
        default:
            break;
    }

    switch (Log::config.print_log_to_stdout) {
        case 0:
            break;
        case 1: {
            if (logentry.loglvl == loglevel::failure) {
                std::cerr << message << std::flush;
            }
        }
        case 2: {
            if (logentry.loglvl == loglevel::warn) {
                std::cerr << message << std::flush;
            }
        }

        case 3: {
                if (logentry.loglvl == loglevel::intrest) {
                    std::cout << message << std::flush;
                }
            }
        case 4: {
                if (logentry.loglvl == loglevel::debug) {
                    std::cout << message << std::flush;
                }
            }
    }

    if (instance.allLog.is_open())
        instance.allLog << message << std::flush;
}

std::string Log::messageFromLogEntry(const struct logentry& _logentry, int loglevel) {
    std::stringstream message;

    std::string logCategory;
    switch (loglevel) {
        case logfile::error:
            logCategory = " error";
            break;
        case logfile::func:
            logCategory = " func ";
            break;
        case logfile::sql:
            logCategory = "  sql ";
            break;
        case logfile::log_access:
            logCategory = "access";
            break;
        default:
            logCategory = " info ";
    }

    std::string logLevelStr;
    switch (_logentry.loglvl) {
        case debug:
            logLevelStr = "DEBUG";
            break;
        case intrest:
            logLevelStr = "INTER";
            break;
        case warn:
            logLevelStr = " WARN";
            break;
        case failure:
            logLevelStr = "FAIL!";
            break;
        default:
            logLevelStr = "LEVEL";
    }
    auto explode = [](const std::string& s, const char& c) -> std::vector<std::string> {
        std::string buff{""};
        std::vector<std::string> v;

        for(auto n:s)
        {
            if(n != c) buff+=n; else
            if(n == c && buff != "") { v.push_back(buff); buff = ""; }
        }
        if(buff != "") v.push_back(buff);

        return v;
    };

    std::vector<std::string> msg{explode(_logentry.message, '\n')};
    std::string new_message;
    for (int i = 0; (i < Log::config.entry_max_lines) && (msg.size() > i); ++i) {
        if (i > 0)
            new_message += "\n\t";
        new_message += msg.at(i);
    }
    if (Log::config.entry_max_lines < msg.size()) {
        if (Log::config.entry_max_lines > 1)
            new_message += "\n\t";
        new_message += "...";
    }
    message << "[ " << _logentry.timestamp << " ] - ";
    message << "[ " << logLevelStr << " ] - ";
    message << "[ " << logCategory << " ] - ";
    message << "thrown at " << _logentry.thrownAt << " | ";
    message << "interface " << _logentry.interface << " | ";
    message << "message: " << new_message << "\n";

    return message.str();
}

void Log::setLogLevel(loglevel level) {
    Log::logstate = level;
}

void Log::set_log_config(struct log_config _config) {
    Log::config = _config;
}

