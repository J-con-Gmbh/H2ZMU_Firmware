//
// Created by jriessner on 13.05.2022.
//

#ifndef H2ZMU_2_E_USER_H
#define H2ZMU_2_E_USER_H


#include <string>

struct user {
    int id = -1;
    std::string username = "";
    size_t passwordhash = 0;
    int groupId = 0;
    long lastlogin = 0; //format: unixTs
    bool deleted = false;
};

#endif //H2ZMU_2_E_USER_H
