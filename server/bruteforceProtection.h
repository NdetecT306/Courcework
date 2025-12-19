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
    system_clock::time_point lastattempt;
    system_clock::time_point blockuntil;
};
class BruteForceProtection {
private:
    mutex lockmutex;
    map<string, LoginAttempt> loginattempts;
public:
    bool canAttemptLogin(const string& login);
    void recordFailedAttempt(const string& login);
    void recordSuccessfulAttempt(const string& login);
    int getRemainingBlockTime(const string& login);
};

#endif
