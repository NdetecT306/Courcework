#include "SHA.h"
string generateSalt() {
    auto now = system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = duration_cast<milliseconds>(duration).count();
    mt19937 generator(millis);
    uniform_int_distribution<int> dist_num(100000, 999999);
    uniform_int_distribution<char> dist_char('a', 'z');
    uniform_int_distribution<char> dist_char_upper('A', 'Z');
    string salt;
    for (int i = 0; i < 8; i++) {
        if (i % 3 == 0) {
            salt += dist_char(generator);
        } else if (i % 3 == 1) {
            salt += dist_char_upper(generator);
        } else {
            salt += to_string(dist_num(generator) % 10);
        }
    }
    salt += to_string(millis % 10000);
    return salt;
}
string addSalt(const string& password, const string& salt) {
    string reversed_salt = salt;
    reverse(reversed_salt.begin(), reversed_salt.end());
    return salt + password + reversed_salt;
}
string SHAWithSalt(const string& password) {
    string salt = generateSalt();
    string salted_password = addSalt(password, salt);
    string hash_result = SHA(salted_password);
    return hash_result + ":" + salt;
}
bool verifyPassword(const string& input_password, const string& stored_hash_with_salt) {
    size_t separator_pos = stored_hash_with_salt.find(':');
    if (separator_pos == string::npos) {
        return false;  
    }
    string stored_hash = stored_hash_with_salt.substr(0, separator_pos);
    string stored_salt = stored_hash_with_salt.substr(separator_pos + 1);
    string salted_input = addSalt(input_password, stored_salt);
    string input_hash = SHA(salted_input);
    return input_hash == stored_hash;
}
string step1(const string& psw) {
    string dwovid; 
    for (char c : psw) { 
        bitset<8> binaryChar(c);
        dwovid += binaryChar.to_string();
    }
    int len = psw.length() * 8;  
    dwovid += "1"; 
    while((dwovid.length() % 512) != 448) { 
        dwovid += "0";
    }
    bitset<64> binary_len(len); 
    string len_str = binary_len.to_string();  
    dwovid += len_str;
    return dwovid;
}
vector<unsigned int> step2 = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};
vector<unsigned int> step3 = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};
unsigned int rightrotate(unsigned int x, int n) {
    return (x >> n) | (x << (32 - n));
}
unsigned int rightshift(unsigned int x, int n) {
    return x >> n;
}
unsigned int ch(unsigned int e, unsigned int f, unsigned int g) {
    return (e & f) ^ ((~e) & g);
}
unsigned int maj(unsigned int a, unsigned int b, unsigned int c) {
    return (a & b) ^ (a & c) ^ (b & c);
}
unsigned int s0ForStep6(unsigned int a) {
    return rightrotate(a, 2) ^ rightrotate(a, 13) ^ rightrotate(a, 22);
}
unsigned int s1ForStep6(unsigned int e) {
    return rightrotate(e, 6) ^ rightrotate(e, 11) ^ rightrotate(e, 25);
}
unsigned int s0ForStep5(unsigned int w) {
    return rightrotate(w, 7) ^ rightrotate(w, 18) ^ rightshift(w, 3);
}
unsigned int s1ForStep5(unsigned int w) {
    return rightrotate(w, 17) ^ rightrotate(w, 19) ^ rightshift(w, 10);
}
void step5(const string& block, vector<unsigned int>& w) {
    w.clear(); 
    for (size_t i = 0; i < 512; i += 32) {
        if (i + 32 <= block.length()) {
            string part = block.substr(i, 32);
            w.push_back(bitset<32>(part).to_ulong());
        }
    }
    while (w.size() < 16) w.push_back(0);
    w.resize(64);
    for(int i = 16; i < 64; i++) {
        w[i] = s1ForStep5(w[i-2]) + w[i-7] + s0ForStep5(w[i-15]) + w[i-16];
    }
}
void step6(vector<unsigned int>& w, vector<unsigned int>& H) {
    vector<unsigned int> letters = H;   
    for(int i = 0; i < 64; i++) {
        unsigned int temp1 = letters[7] + s1ForStep6(letters[4]) + ch(letters[4], letters[5], letters[6]) + step3[i] + w[i];
        unsigned int temp2 = s0ForStep6(letters[0]) + maj(letters[0], letters[1], letters[2]);
        letters[7] = letters[6];
        letters[6] = letters[5];
        letters[5] = letters[4];
        letters[4] = letters[3] + temp1;
        letters[3] = letters[2];
        letters[2] = letters[1];
        letters[1] = letters[0];
        letters[0] = temp1 + temp2;
    }
    for(int i = 0; i < 8; i++) {
        H[i] = H[i] + letters[i];
    }
}
void step4(const string& padded_message, vector<unsigned int>& H) {
    H = step2;
    size_t num_blocks = padded_message.length() / 512;
    for (size_t block_num = 0; block_num < num_blocks; block_num++) {
        string block = padded_message.substr(block_num * 512, 512);
        vector<unsigned int> w;
        step5(block, w);
        step6(w, H);
    }
}
string step7(const vector<unsigned int>& H) {
    stringstream ss;
    for(unsigned int val : H) {
        ss << hex << setw(8) << setfill('0') << val;
    }
    return ss.str();
}
string SHA(const string& str) {
    string padded_message = step1(str);
    vector<unsigned int> H;
    step4(padded_message, H);
    string hash = step7(H);
    return hash;
}
