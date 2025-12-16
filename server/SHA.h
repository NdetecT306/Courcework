#ifndef SHA_H
#define SHA_H
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <bitset>
#include <vector>
#include <algorithm>
using namespace std;
using namespace std::chrono;

string SHA(const string& str);
string generateSalt();
string addSalt(const string& password, const string& salt);
string SHAWithSalt(const string& password);
bool verifyPassword(const string& input_password, const string& stored_hash_with_salt);

#endif 
