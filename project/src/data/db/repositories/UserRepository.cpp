//
// Created by jriessner on 13.05.2022.
//

#include <iostream>
#include <algorithm>

#include "data/db/repositories/UserRepository.h"
#include "Log.h"

std::shared_ptr<UserRepository> UserRepository::instance;

std::map<int, struct user> UserRepository::getAllUser() {
    return allUser;
}

bool UserRepository::createUser(struct user *_user) {

    if (_user->username.empty()) {

        return false;
    }
    for (const auto &item : allUser) {
        if (item.second.username == _user->username) {
            return false;
        }
    }

    std::string sql = "INSERT INTO users ( username, passwordhash, fk_usergroup, lastlogin, deleted) VALUES (";
    sql += "'" + _user->username + "', '" +
           std::to_string(_user->passwordhash) + "', '" +
           std::to_string(_user->groupId) + "', '" +
           std::to_string(_user->lastlogin) + "', '" +
           std::to_string(_user->deleted);
    sql += "');";

    bool sqlSuccess = databaseService->executeSql(sql);
    if (sqlSuccess) {
        std::string sqlId = "SELECT id FROM users WHERE username = '";
        sqlId += _user->username;
        sqlId += "';";

        std::string query = databaseService->executeSqlReturn(sqlId);
        std::string id = utils::strings::getValueFromQuery(query, "id", ";");
        _user->id = std::stoi(id);


    } else {

    }
    allUser[_user->id] = *_user;

    return sqlSuccess;
}

bool UserRepository::updateUser(const struct user& _user) {

    std::string sql = "UPDATE users SET ";
    int iLength = (int) sql.length();

    struct user oldUser = allUser[_user.id];

    if (_user.username != oldUser.username) {
        sql += "username = '" + _user.username + "', ";
    }
    if (_user.passwordhash != oldUser.passwordhash) {
        sql += "passwordhash = '" + std::to_string(_user.passwordhash) + "', ";
    }
    if (_user.groupId != oldUser.groupId) {
        sql += "usergroup = '" + std::to_string(_user.groupId) + "', ";
    }
    if (_user.lastlogin != oldUser.lastlogin) {
        sql += "lastlogin = '" + std::to_string(_user.lastlogin) + "', ";
    }
    if (_user.deleted != oldUser.deleted) {
        sql += "deleted = '" + std::to_string(_user.deleted) + "', ";
    }

    bool sqlSuccess;
    if (sql.length() > iLength) {
        sql = utils::strings::trimBy(sql, 2);
        sql += " WHERE id = '" + std::to_string(_user.id) + "';";

        sqlSuccess = databaseService->executeSql(sql);
        if (sqlSuccess) {
            allUser[_user.id] = _user;
        }

        return sqlSuccess;
    } else {
        return true;
    }
}

struct user UserRepository::getUserById(int id) {
    for (const auto &user: allUser)
    {
        if (user.second.id == id)
        {
            return user.second;
        }
    }

    return {};
}

bool UserRepository::loadAll() {
    std::string sql = "SELECT * FROM users";

    std::string ret = databaseService->executeSqlReturn(sql);

    size_t pos = 0;
    std::string token;
    while ((pos = ret.find(';')) != std::string::npos) {
        token = ret.substr(0, pos);
        struct user _user = getUserFromQuery(token);
        allUser[_user.id] = _user;
        ret.erase(0, pos + 1);
    }

    return true;
}

struct user UserRepository::getUserFromQuery(const std::string& query) {

    struct user _user;
    _user.id = std::stoi(utils::strings::getValueFromQuery(query, "id", ","));
    _user.username = utils::strings::getValueFromQuery(query, "username", ",");
    std::stringstream pwHash(utils::strings::getValueFromQuery(query, "passwordhash", ","));
    pwHash >> _user.passwordhash;
    _user.groupId = std::stoi(utils::strings::getValueFromQuery(query, "fk_usergroup", ","));
    _user.lastlogin = std::stoi(utils::strings::getValueFromQuery(query, "lastlogin", ","));
    _user.deleted = ( "1" == utils::strings::getValueFromQuery(query, "lastlogin", ","));

    return _user;
}

std::string UserRepository::dumpUser(const struct user& _user) {
    std::stringstream dump;
    dump << "struct user {\n"
                 << "\tid:\t\t" << _user.id
                 << "\n\tname:\t" << _user.username
                 << "\n\tgroup:\t" << _user.groupId
                 << "\n\tdeleted:" << _user.deleted
                 << "\n}" << std::endl;

    return dump.str();
}

struct user UserRepository::getUserByName(const std::string& name) {
    for (const auto &item : allUser) {
        if (item.second.username == name) return item.second;
    }

    return {};
}

bool UserRepository::deleteUser(struct user* _user) {
    _user->deleted = true;
    return this->updateUser(*_user);
}

bool UserRepository::createUserWithPassword(struct user *_user, std::string password) {
    std::hash<std::string> hash;
    _user->passwordhash = this->hashString(password);

    return this->createUser(_user);
}

bool UserRepository::updateUserPassword(user *_user, std::string password) {
    /// TODO auf sonstige Änderungen testen -> nur Passwortänderung erlauben
    _user->passwordhash = this->hashString(password);
    return this->updateUser(*_user);
}

size_t UserRepository::hashString(const std::string& toHash) {
    std::hash<std::string> hash;
    return hash(toHash);
}