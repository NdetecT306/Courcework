#include "bruteforceProtection.h"
#include <iostream>

bool BruteForceProtection::canAttemptLogin(const std::string& login) {
    lock_guard<mutex> lock(lockmutex);
    auto now = system_clock::now();
    auto it = loginattempts.find(login);
    if (it != loginattempts.end()) {
        // Если пользователь заблокирован
        if (now < it->second.blockuntil) {
            return false; 
        }
        // Если прошло больше 5 минут с последней попытки, сбрасываем счетчик
        auto timesincelast = duration_cast<minutes>(now - it->second.lastattempt);
        if (timesincelast > minutes(5)) {
            loginattempts.erase(it);
            return true;
        }
    }
    return true;
}
void BruteForceProtection::recordFailedAttempt(const string& login) {
    lock_guard<mutex> lock(lockmutex);
    auto now = system_clock::now();
    auto& attempt = loginattempts[login];
    attempt.attempts++;
    attempt.lastattempt = now;
    if (attempt.attempts >= 4) {
        int delayseconds = (attempt.attempts - 3) * 5;
        attempt.blockuntil = now + seconds(delayseconds);
    }
}
void BruteForceProtection::recordSuccessfulAttempt(const string& login) {
    lock_guard<mutex> lock(lockmutex);
    loginattempts.erase(login);
}
int BruteForceProtection::getRemainingBlockTime(const string& login) {
    lock_guard<mutex> lock(lockmutex);
    auto it = loginattempts.find(login);
    if (it != loginattempts.end()) {
        auto now = system_clock::now();
        if (now < it->second.blockuntil) {
            auto remaining = duration_cast<seconds>(it->second.blockuntil - now);
            return remaining.count();
        }
    }
    return 0;
}
