//
// Created by jriessner on 13.05.2022.
//

#ifndef H2ZMU_2_USERREPOSITORY_H
#define H2ZMU_2_USERREPOSITORY_H


#include <list>
#include <map>
#include "data/db/entities/e_User.h"
#include "string_processing.h"
#include "data/db/DatabaseService.h"
#include "Repository.h"

class UserRepository: public Repository {
    std::map<int, struct user> allUser;

    struct user getUserFromQuery(const std::string& query);

    size_t hashString(const std::string& toHash);

public:
    static std::shared_ptr<UserRepository> instance;

    UserRepository() : Repository("users") {}
    bool loadAll() override;
    struct user getUserById(int id);
    struct user getUserByName(const std::string& name);
    bool updateUser(const struct user& _user);
    bool updateUserPassword(struct user* _user, std::string password);
    bool createUser(struct user *_user);
    bool createUserWithPassword(struct user *_user, std::string password);
    bool deleteUser(struct user* _user);
    std::map<int, struct user> getAllUser();
    static std::string dumpUser(const struct user& _user);
};


#endif //H2ZMU_2_USERREPOSITORY_H
