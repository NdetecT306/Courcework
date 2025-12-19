#include "userDatabase.h"
#include "SHA.h"
#include <iostream>
UserDatabaseManager::UserDatabaseManager() : connection(
    "host=172.17.0.1 "
    "port=5432 "
    "dbname=userbd "
    "user=admin306 "
    "password=ILoveMyCreation"
) {}
bool UserDatabaseManager::authenticateUser(const string& login, const string& password, BruteForceProtection& bfprotection) {
    // Проверяем блокировку перед аутентификацией
    if (!bfprotection.canAttemptLogin(login)) {
        int remainingtime = bfprotection.getRemainingBlockTime(login);
        cerr << "Пользователь " << login << " заблокирован. Осталось: " << remainingtime << " сек" << endl;
        return false;
    }
    lock_guard<mutex> lock(dbmutex);
    try {
        pqxx::work txn(connection);
        auto result = txn.exec_params(
            "SELECT password_hash FROM users WHERE login = $1",
            login
        );
        txn.commit();
        if (result.empty()) {
            bfprotection.recordFailedAttempt(login); // Запоминаем неудачную попытку
            return false; 
        }
        string storedhashwithsalt = result[0]["password_hash"].as<string>();
        bool authenticated = verifyPassword(password, storedhashwithsalt);
        if (authenticated) {
            bfprotection.recordSuccessfulAttempt(login); 
            pqxx::work txn2(connection);
            txn2.exec_params(
                "UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE login = $1",
                login
            );
            txn2.commit();
        } else {
            bfprotection.recordFailedAttempt(login);
        }
        return authenticated;
    } catch (const std::exception& e) {
        cerr << "Ошибка аутентификации: " << e.what() << endl;
        bfprotection.recordFailedAttempt(login); 
        return false;
    }
}
bool UserDatabaseManager::registerUser(const string& login, const string& password) {
    lock_guard<mutex> lock(dbmutex);
    try {
        string hashwithsalt = SHAWithSalt(password);
        pqxx::work txn(connection);
        txn.exec_params(
            "INSERT INTO users (login, password_hash) VALUES ($1, $2)",
            login, hashwithsalt
        );
        txn.commit();
        return true;
    } catch (const exception& e) {
        cerr << "Ошибка регистрации: " << e.what() << endl;
        return false;
    }
}
bool UserDatabaseManager::userExists(const string& login) {
    lock_guard<mutex> lock(dbmutex);
    try {
        pqxx::work txn(connection);
        auto result = txn.exec_params(
            "SELECT COUNT(*) FROM users WHERE login = $1",
            login
        );
        txn.commit();
        return result[0][0].as<int>() > 0;
    } catch (const exception& e) {
        cerr << "Ошибка в получении данных пользователя: " << e.what() << endl;
        return false;
    }
}
json UserDatabaseManager::getUserProfile(const string& login) {
    lock_guard<mutex> lock(dbmutex);
    try {
        pqxx::work txn(connection);
        auto result = txn.exec_params(
            "SELECT login, nickname, status, birthdate, photo_path "
            "FROM user_profiles WHERE login = $1",
            login
        );
        txn.commit();
        if (result.empty()) {
            return {{"login", login}, {"nickname", login}};
        }
        json profile;
        profile["login"] = result[0]["login"].as<string>();
        profile["nickname"] = result[0]["nickname"].as<string>();
        profile["status"] = result[0]["status"].as<string>();
        profile["birthdate"] = result[0]["birthdate"].as<string>();
        profile["photo_path"] = result[0]["photo_path"].as<string>();
        return profile;
    } catch (const exception& e) {
        cerr << "Ошибка в получении профиля: " << e.what() << endl;
        return {{"error", e.what()}};
    }
}
bool UserDatabaseManager::saveUserProfile(const json& profile) {
    lock_guard<mutex> lock(dbmutex);
    try {
        string login = profile["login"].get<string>();
        string nickname = profile["nickname"].get<string>();
        string status = profile["status"].get<string>();
        string birthdate = profile["birthdate"].get<string>();
        string photopath = profile["photo_path"].get<string>();
        pqxx::work txncheck(connection);
        auto check = txncheck.exec_params(
            "SELECT COUNT(*) FROM user_profiles WHERE login = $1",
            login
        );
        txncheck.commit();
        if (check[0][0].as<int>() > 0) {
            pqxx::work txn(connection);
            txn.exec_params(
                "UPDATE user_profiles SET nickname = $1, status = $2, "
                "birthdate = $3, photo_path = $4 WHERE login = $5",
                nickname,
                status,
                birthdate,
                photopath,
                login
            );
            txn.commit();
        } else {
            pqxx::work txn(connection);
            txn.exec_params(
                "INSERT INTO user_profiles (login, nickname, status, birthdate, photo_path) "
                "VALUES ($1, $2, $3, $4, $5)",
                login,
                nickname,
                status,
                birthdate,
                photopath
            );
            txn.commit();
        }
        return true;
    } catch (const exception& e) {
        cerr << "Ошибка в сохранении профиля: " << e.what() << endl;
        return false;
    }
}
bool UserDatabaseManager::verifyPassword(const string& inputpassword, const string& storedhashwithsalt) {
    return ::verifyPassword(inputpassword, storedhashwithsalt);
}
string UserDatabaseManager::SHAWithSalt(const string& password) {
    return ::SHAWithSalt(password);
}
