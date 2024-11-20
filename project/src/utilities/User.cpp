/*
 * Filename: User.cpp
 * Path: h2zmu-axcf/project/src/utilities
 * Created Date: Monday, July 29th 2024, 9:13:57 am
 * Author: PaulEmeriau
 */

#include "User.h"

User::User()
{
    // TODO: log of authentification

    // * if compiled with -DCMAKE_BUILD_TYPE=Debug you will be automatically logged as admin.
    #ifndef NDEBUG
        this->_level = ADMIN;
        std::cout << "debug active logged as admin" << std::endl;
        return;
    #endif

    std::map<std::string, std::string> userPassword = 
        confFileParser("user.conf");
    std::string user;
    std::string password;
    std::hash<std::string> hashString;

    std::cout << "Enter username: ";                 // * get the creditentials
    std::getline(std::cin, user);
    std::cout << "Enter password: ";
    std::getline(std::cin, password);
    for (auto &element : userPassword) {            // * compare the creditentials to the logs info
        if (user.compare(element.first) == 0 &&     // * (the conf file in the machine (user.conf))
            std::to_string(hashString(password)).compare(element.second) == 0) {
            this->assignLevel(user);
            std::cout << "Connected as " << user << std::endl;
            return;
        }
    }
    this->loginError();
}

void User::assignLevel(std::string user)
{
    if (user.compare("user") == 0)
        this->_level = USER;
    else if (user.compare("service") == 0)
        this->_level = SERVICE;
    else if (user.compare("admin") == 0)
        this->_level = ADMIN;
    else
        std::cout << "Login error no role will be attributed" << std::endl;
}

void User::loginError()
{
    std::cout << "Connection failed" << std::endl;
    // TODO : create the logs of the failed connection here
    exit(1);
}

enum level User::getLevel() const
{
    return this->_level;
}

bool User::hasUserRights()
{
    if (this->_level >= USER)
        return true;
    return false;
}

bool User::hasServiceRights()
{
    if (this->_level >= SERVICE)
        return true;
    return false;
}

bool User::hasAdminRights()
{
    if (this->_level == ADMIN)
        return true;
    return false;
}
