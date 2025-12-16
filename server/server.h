#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include "userDatabase.h"
#include "quizDatabase.h"
#include "resourceDatabase.h"
#include "bruteforceProtection.h"
#include "clientSession.h"
using boost::asio::ip::tcp;
using namespace std;

class Server {
private:
    boost::asio::io_context io_context;
    tcp::acceptor acceptor;
    UserDatabaseManager user_db_manager;
    QuizDatabaseManager quiz_db_manager;
    ResourceDatabaseManager resource_db_manager;
    BruteForceProtection bf_protection;
public:
    Server(short port);
    void run();
private:
    void do_accept();
};

#endif
