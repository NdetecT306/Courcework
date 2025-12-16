#include "userDatabase.h"
#include "SHA.h"
#include <iostream>
UserDatabaseManager::UserDatabaseManager() : connection(
    "host=localhost "
    "port=5432 "
    "dbname=userbd "
    "user=admin306 "
    "password=ILoveMyCreation"
) {}
bool UserDatabaseManager::authenticateUser(const string& login, const string& password, BruteForceProtection& bf_protection) {
    // Проверяем блокировку перед аутентификацией
    if (!bf_protection.canAttemptLogin(login)) {
        int remaining_time = bf_protection.getRemainingBlockTime(login);
        cerr << "Пользователь " << login << " заблокирован. Осталось: " << remaining_time << " сек" << endl;
        return false;
    }
    lock_guard<mutex> lock(db_mutex);
    try {
        pqxx::work txn(connection);
        auto result = txn.exec_params(
            "SELECT password_hash FROM users WHERE login = $1",
            login
        );
        txn.commit();
        if (result.empty()) {
            bf_protection.recordFailedAttempt(login); // Запоминаем неудачную попытку
            return false; 
        }
        string stored_hash_with_salt = result[0]["password_hash"].as<string>();
        bool authenticated = verifyPassword(password, stored_hash_with_salt);
        if (authenticated) {
            bf_protection.recordSuccessfulAttempt(login); 
            pqxx::work txn2(connection);
            txn2.exec_params(
                "UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE login = $1",
                login
            );
            txn2.commit();
        } else {
            bf_protection.recordFailedAttempt(login);
        }
        return authenticated;
    } catch (const std::exception& e) {
        cerr << "Ошибка аутентификации: " << e.what() << endl;
        bf_protection.recordFailedAttempt(login); 
        return false;
    }
}
bool UserDatabaseManager::registerUser(const string& login, const string& password) {
    lock_guard<mutex> lock(db_mutex);
    try {
        string hash_with_salt = SHAWithSalt(password);
        pqxx::work txn(connection);
        txn.exec_params(
            "INSERT INTO users (login, password_hash) VALUES ($1, $2)",
            login, hash_with_salt
        );
        txn.commit();
        return true;
    } catch (const exception& e) {
        cerr << "Ошибка регистрации: " << e.what() << endl;
        return false;
    }
}
bool UserDatabaseManager::userExists(const string& login) {
    lock_guard<mutex> lock(db_mutex);
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
    lock_guard<mutex> lock(db_mutex);
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
    lock_guard<mutex> lock(db_mutex);
    try {
        string login = profile["login"].get<string>();
        string nickname = profile["nickname"].get<string>();
        string status = profile["status"].get<string>();
        string birthdate = profile["birthdate"].get<string>();
        string photo_path = profile["photo_path"].get<string>();
        pqxx::work txn_check(connection);
        auto check = txn_check.exec_params(
            "SELECT COUNT(*) FROM user_profiles WHERE login = $1",
            login
        );
        txn_check.commit();
        if (check[0][0].as<int>() > 0) {
            pqxx::work txn(connection);
            txn.exec_params(
                "UPDATE user_profiles SET nickname = $1, status = $2, "
                "birthdate = $3, photo_path = $4 WHERE login = $5",
                nickname,
                status,
                birthdate,
                photo_path,
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
                photo_path
            );
            txn.commit();
        }
        return true;
    } catch (const exception& e) {
        cerr << "Ошибка в сохранении профиля: " << e.what() << endl;
        return false;
    }
}
bool UserDatabaseManager::verifyPassword(const string& input_password, const string& stored_hash_with_salt) {
    return ::verifyPassword(input_password, stored_hash_with_salt);
}
string UserDatabaseManager::SHAWithSalt(const string& password) {
    return ::SHAWithSalt(password);
}
