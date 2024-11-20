//
// Created by jriessner on 12.05.2022.
//

#ifndef H2ZMU_2_LOG_H
#define H2ZMU_2_LOG_H

#define CONF_LOG_DIR "LOG_DIR"

#include <list>
#include <mutex>
#include <fstream>
#include <iomanip>


namespace h2zmu {

// "%Y-%m-%d %T.%S"
inline std::string getTimestamp(std::string format) {

    bool fHasMillis = false;
    if (size_t index = format.find("%S") != std::string::npos) {
        format = format.substr(0, format.length() - index);
        fHasMillis = true;
    }

    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() % 1000;

    std::string smillis;
    if (millis < 10) {
        smillis = "00" + std::to_string(millis);
    } else if (millis < 100) {
        smillis = "0" + std::to_string(millis);
    } else {
        smillis = std::to_string(millis);
    }

    std::stringstream ts;

    time_t now1 = time(nullptr);
    ts << std::put_time(localtime(&now1), format.c_str());
    if (fHasMillis)
        ts << smillis;

    return ts.str();
}


enum logfile {
    info = 0,
    error = 1,
    func = 2,
    sql = 3,
    log_access = 4,
};

enum loglevel {
    debug = 0,
    intrest = 1,
    warn = 2,
    failure = 4,
};

enum interface {
    no_interface = 0,
    hmi = 1,
    modbus_client = 2,
    modbus_server = 3,
    remote_control = 4,
};

struct logentry {
    std::string message;
    int interface = interface::no_interface;
    //int user = Security::getUserFromThread(std::this_thread::get_id()).id;
    std::string thrownAt;
    std::string timestamp = getTimestamp("%Y-%m-%d %T.%S");
    loglevel loglvl = loglevel::debug;
};
struct log_config {
    int entry_max_lines = 10;
    int print_log_to_stdout = 0;
};

class Log{
    std::ofstream allLog;
    std::ofstream errorLog;
    std::ofstream functionLog;
    std::ofstream sqlLog;
    std::ofstream accessLog;
    std::ofstream sensorLog;
    //Logger instance (singleton)

    static bool verbose;

    static Log instance;

    static std::string messageFromLogEntry(const struct logentry& _logentry, int loglevel);

    static loglevel logstate;

    static log_config config;

public:
    Log();
    static void open( const std::string& logDir);
    static void setLogLevel(loglevel level);
    static void close();
    // write message
    static void write( const struct logentry& logentry);
    static void error( struct logentry logentry);
    static void logfunc( const struct logentry& logentry);

    static void write(const struct logentry& logentry, int _logfile);

    static void set_log_config(struct log_config _config);

};

}

#endif //H2ZMU_2_LOG_H
