#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <mutex>
#include <thread>
#include <iostream>
using namespace std;
using json = nlohmann::json;
using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

class NetworkClient {
public:
    NetworkClient(const string& host = "localhost", short port = 8080,const string& cert_file = "server.crt");
    ~NetworkClient();
    bool connect();
    void disconnect();
    bool is_connected() const;
    json send_request(const json& request);
    json login(const string& username, const string& password);
    json register_user(const string& username, const string& password);
    json get_profile(const string& username);
    json save_profile(const json& profile);
    json check_connection();
    json get_categories();
    json get_questions(int categoryId);
    json check_answers(const string& userLogin, int categoryId, const json& answers);
    json get_statistics(const string& userLogin);
    json get_resource_categories();
    json get_resources_by_category(int categoryId);
    json get_all_resources();
    json get_resources_with_categories();
    
private:
    boost::asio::io_context io_context;
    ssl::context ssl_context;
    ssl::stream<tcp::socket> socket;
    thread io_thread;
    string serverhost;
    short serverport;
    string cert_file;
    bool connected;
    mutable mutex socketmutex;
};

extern NetworkClient* g_network_client;

#endif
