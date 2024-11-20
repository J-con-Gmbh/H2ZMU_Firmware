/*
 * Filename: User.h
 * Path: h2zmu-axcf/project/src/utilities
 * Created Date: Monday, July 29th 2024, 9:13:57 am
 * Author: PaulEmeriau
 */

#ifndef USER_H
    #define USER_H

    #include <iostream>
    #include <functional>
    #include <string>
    #include <cstring>
    #include "utilities/tools/configParser.h"

enum level {
    USER,
    SERVICE,
    ADMIN
};

class User {
    private:
        enum level _level;
    
    public:
        User();

        enum level getLevel() const;

        void assignLevel(std::string user);
        void loginError();

        bool hasUserRights();
        bool hasServiceRights();
        bool hasAdminRights();
};

#endif
