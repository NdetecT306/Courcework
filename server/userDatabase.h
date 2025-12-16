#ifndef USERDATABASEMANAGER_H
#define USERDATABASEMANAGER_H
#include <string>
#include <mutex>
#include <pqxx/pqxx>
#include "json.hpp"
#include "bruteforceProtection.h"
using json = nlohmann::json;
using namespace std;

class UserDatabaseManager {
private:
    pqxx::connection connection;
    mutex db_mutex;
public:
    UserDatabaseManager();
    bool authenticateUser(const string& login, const string& password, BruteForceProtection& bf_protection);
    bool registerUser(const string& login, const string& password);
    bool userExists(const string& login);
    json getUserProfile(const string& login);
    bool saveUserProfile(const json& profile);
private:
    bool verifyPassword(const string& input_password, const string& stored_hash_with_salt);
    string SHAWithSalt(const string& password);
};

#endif
