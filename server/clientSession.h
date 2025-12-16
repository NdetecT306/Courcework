#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H
#include <boost/asio.hpp>
#include <array>
#include <memory>
#include "userDatabase.h"
#include "quizDatabase.h"
#include "resourceDatabase.h"
#include "bruteforceProtection.h"
using boost::asio::ip::tcp;
using json = nlohmann::json;
using namespace std;

class ClientSession : public std::enable_shared_from_this<ClientSession> {
private:
    tcp::socket socket;
    UserDatabaseManager& user_db;
    QuizDatabaseManager& quiz_db;
    ResourceDatabaseManager& resource_db;
    BruteForceProtection& bf_protection;
    array<char, 8192> buffer;
public:
    ClientSession(tcp::socket sock, UserDatabaseManager& user_db_manager, QuizDatabaseManager& quiz_db_manager, ResourceDatabaseManager& resource_db_manager,BruteForceProtection& bf_protector);
    void start();   
private:
    void do_read();
    void process_request(const string& request_str);
    void send_response(const json& response);
};

#endif
