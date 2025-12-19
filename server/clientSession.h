#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>
#include "userDatabase.h"
#include "quizDatabase.h"
#include "resourceDatabase.h"
#include "bruteforceProtection.h"
#include <nlohmann/json.hpp>
#include <iostream>
using json = nlohmann::json;
using boost::asio::ip::tcp;

class ClientSession : public std::enable_shared_from_this<ClientSession> {
private:
    void do_read();
    void process_request(const std::string& requeststr);
    void send_response(const json& response);
    void handle_handshake(const boost::system::error_code& error);
    boost::asio::ssl::stream<tcp::socket> socket;
    UserDatabaseManager& userdb;
    QuizDatabaseManager& quizdb;
    ResourceDatabaseManager& resourcedb;
    BruteForceProtection& bfprotection;
    enum { max_length = 4096 };
    char buffer[max_length];
public:
    ClientSession(boost::asio::ssl::stream<tcp::socket> socket, UserDatabaseManager& userdbmanager,QuizDatabaseManager& quizdbmanager,
                  ResourceDatabaseManager& resourcedbmanager,BruteForceProtection& bfprotector);
    void start();
};
#endif
