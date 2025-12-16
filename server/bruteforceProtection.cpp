#include "bruteforceProtection.h"
#include <iostream>

bool BruteForceProtection::canAttemptLogin(const std::string& login) {
    lock_guard<mutex> lock(lock_mutex);
    auto now = system_clock::now();
    auto it = login_attempts.find(login);
    if (it != login_attempts.end()) {
        // Если пользователь заблокирован
        if (now < it->second.block_until) {
            return false; 
        }
        // Если прошло больше 5 минут с последней попытки, сбрасываем счетчик
        auto time_since_last = duration_cast<minutes>(now - it->second.last_attempt);
        if (time_since_last > minutes(5)) {
            login_attempts.erase(it);
            return true;
        }
    }
    return true;
}
void BruteForceProtection::recordFailedAttempt(const string& login) {
    lock_guard<mutex> lock(lock_mutex);
    auto now = system_clock::now();
    auto& attempt = login_attempts[login];
    attempt.attempts++;
    attempt.last_attempt = now;
    if (attempt.attempts >= 4) {
        int delay_seconds = (attempt.attempts - 3) * 5;
        attempt.block_until = now + seconds(delay_seconds);
    }
}
void BruteForceProtection::recordSuccessfulAttempt(const string& login) {
    lock_guard<mutex> lock(lock_mutex);
    login_attempts.erase(login);
}
int BruteForceProtection::getRemainingBlockTime(const string& login) {
    lock_guard<mutex> lock(lock_mutex);
    auto it = login_attempts.find(login);
    if (it != login_attempts.end()) {
        auto now = system_clock::now();
        if (now < it->second.block_until) {
            auto remaining = duration_cast<seconds>(it->second.block_until - now);
            return remaining.count();
        }
    }
    return 0;
}
