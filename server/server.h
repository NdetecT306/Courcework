#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>
#include <iostream>
#include "userDatabase.h"
#include "quizDatabase.h"
#include "resourceDatabase.h"
#include "bruteforceProtection.h"
using boost::asio::ip::tcp;
using namespace std;

class ClientSession;
class Server {
private:
    void do_accept();
    void load_certificates();
    boost::asio::io_context iocontext;
    boost::asio::ssl::context ssl_context;
    tcp::acceptor acceptor;
    UserDatabaseManager userdbmanager;
    QuizDatabaseManager quizdbmanager;
    ResourceDatabaseManager resourcedbmanager;
    BruteForceProtection bfprotection;
    string cert_file;
    string key_file;
public:
    Server(short port, const string& cert_file, const string& key_file);
    void run();
};

#endif
