#include <iostream>
#include <thread>

#include "lib/include/conf/Config.h"
#include "lib/include/rgfc/RgfCorr.h"
#include "lib/include/rgfc/RgfH2.h"
#include "lib/include/rgfc/RgfN2.h"
#include "lib/include/epoch_time.h"
#include "lib/include/sys/dump_stacktrace.h"
#include "lib/include/string_processing.h"
#include "lib/include/geom/LinearRegression.h"
#include "lib/include/sys/FileHandler.h"
#include "lib/include/queue/Queue.h"


void stacktrace_test1();
void stacktrace_test2();
void stacktrace_test3();
void stacktrace_test4();


struct queueable {
    int a = 1;
    float b = 1.5;
    std::string c = "abc";
};

void queue_test(const std::string& ident, utils::Queue<queueable> &q, u_int sub, int sleep);
void queue_test_all(const std::string& ident, utils::Queue<queueable> &q, u_int sub, int sleep);

std::mutex p_mutex;

int main() {

    utils::Config conf;
    conf.loadConfigFile("/home/jriessner/Arbeit/Projekte/H2Well/Code/Cpp/h2zmu_v1/files/config.txt");
    std::cout << conf.dumpConfig() << std::endl;


    std::shared_ptr<RgfH2> rgfH2Ptr = std::make_shared<RgfH2>();
    std::shared_ptr<RgfCorr> rgfPtr = std::static_pointer_cast<RgfCorr>(rgfH2Ptr);

    RgfCorr::setGlobalSharedPtr(medium::H2);

    std::cout << rgfH2Ptr->convertNormVolToKg(100) << std::endl;
    std::cout << rgfPtr->convertNormVolToKg(100) << std::endl;
    std::cout << RgfCorr::instance->convertNormVolToKg(100) << std::endl;

    std::shared_ptr<RgfN2> rgfN2Ptr = std::make_shared<RgfN2>();
    rgfPtr = std::static_pointer_cast<RgfCorr>(rgfN2Ptr);

    RgfCorr::setGlobalSharedPtr(medium::N2);

    std::cout << rgfN2Ptr->convertNormVolToKg(100) << std::endl;
    std::cout << rgfPtr->convertNormVolToKg(100) << std::endl;
    std::cout << RgfCorr::instance->convertNormVolToKg(100) << std::endl;


    std::cout << utils::epoch_time::getTimestamp("%Y-%m-%d %T.%S") << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    auto ts = utils::epoch_time::getUnixTimestamp();
    std::cout << utils::epoch_time::formatTimestamp(ts) << std::endl;
    std::cout << utils::epoch_time::formatTimestamp(ts, "%Y-%m-%d %T") << std::endl;
    std::cout << utils::epoch_time::getTimestamp("%Y-%m-%d %T.%S") << std::endl;

    std::cout << utils::sys::dump_stacktrace() << std::flush;

    std::cout << std::endl << "Hallo Test dies das" << std::endl;
    for (const auto &item: utils::strings::splitString("Hallo Test dies das", " ")) {
        std::cout << item << "-";
    }
    std::cout << std::endl << std::endl << "Hallo;Test;dies;das" << std::endl;
    for (const auto &item: utils::strings::splitString("Hallo;Test;dies;das", ";d")) {
        std::cout << item << "-";
    }
    std::cout << std::endl;

    utils::geom::LinearRegression lr;

    stacktrace_test1();

    FileHandler::saveFile("../test_edit.conf", "");


    utils::Config cnf;
    cnf.loadConfig("../test_edit.conf", true);
    cnf.setValue("1", "a");
    cnf.setValue("2", "b");
    cnf.setValue("3", "c");
    cnf.persistConfig();


    utils::Config cnf1;
    cnf1.loadConfig("../test_edit.conf");
    std::cout << cnf1.dumpConfig() << std::endl;


    utils::Queue<queueable> test;
    test.setMaxItems(20);

    u_int sub1 = test.subscribe();
    /*
    u_int sub2 = test.subscribe();
    u_int sub3 = test.subscribe();
    u_int sub4 = test.subscribe();
*/
    std::thread t1(queue_test, "t1", std::ref(test), sub1, 110);
    /*std::thread t2(queue_test, "t2", std::ref(test), sub2, 1000);
    std::thread t3(queue_test, "t3", std::ref(test), sub3, 2000);
    std::thread t4(queue_test_all, "t4", std::ref(test), sub4, 7000);
*/
    for (int i = 0; i < 100; ++i) {
        queueable qa = {.a = i, .c = "item_" + std::to_string(i + 1)};
        u_int index_at = test.add(qa);
        p_mutex.lock();
        std::cout << "inserted at " << index_at << " => item_" << std::to_string(i + 1) << std::endl;
        p_mutex.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds (100));
    }

    t1.join();
    /*
    t2.join();
    t3.join();
    t4.join();
*/
    return 0;
}


void stacktrace_test1() {
    stacktrace_test2();
}

void stacktrace_test2() {
    stacktrace_test3();
}

void stacktrace_test3() {
    stacktrace_test4();
}

void stacktrace_test4() {
    std::cout << utils::sys::dump_stacktrace() << std::endl;
}

void queue_test(const std::string& ident, utils::Queue<queueable> &q, u_int sub, int sleep) {

    for (int i = 0; i < 100;) {
        if (q.hasNew(sub)) {

            auto t = q.getNext(sub);
            if (!std::get<bool>(t)) {
                continue;
            }
            p_mutex.lock();
            std::cout << ident << ": " << std::get<queueable>(t).c << std::endl;
            p_mutex.unlock();
            i++;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }
}

void queue_test_all(const std::string& ident, utils::Queue<queueable> &q, u_int sub, int sleep) {

    for (int i = 0; i < 10;) {
        if (q.hasNew(sub)) {

            auto t = q.getNew(sub);
            p_mutex.lock();
            std::cout << ident << ": ";
            for (const auto &item: t) {
                std::cout << "[" << item.a << ": " << item.c << "] ";
            }
            std::cout << std::endl;
            p_mutex.unlock();
            i += (int) t.size();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }
}