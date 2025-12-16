#ifndef BRUTEFORCEPROTECTION_H
#define BRUTEFORCEPROTECTION_H
#include <string>
#include <mutex>
#include <map>
#include <chrono>
using namespace std;
using namespace std::chrono;

struct LoginAttempt {
    int attempts;
    system_clock::time_point last_attempt;
    system_clock::time_point block_until;
};
class BruteForceProtection {
private:
    mutex lock_mutex;
    map<string, LoginAttempt> login_attempts;
public:
    bool canAttemptLogin(const string& login);
    void recordFailedAttempt(const string& login);
    void recordSuccessfulAttempt(const string& login);
    int getRemainingBlockTime(const string& login);
};

#endif
