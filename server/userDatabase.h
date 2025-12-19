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
    mutex dbmutex;
    bool verifyPassword(const string& inputpassword, const string& storedhashwithsalt);
    string SHAWithSalt(const string& password);
public:
    UserDatabaseManager();
    bool authenticateUser(const string& login, const string& password, BruteForceProtection& bfprotection);
    bool registerUser(const string& login, const string& password);
    bool userExists(const string& login);
    json getUserProfile(const string& login);
    bool saveUserProfile(const json& profile);
};
#endif
