#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <mutex>
#include <memory>
using namespace std;
using json = nlohmann::json;
using boost::asio::ip::tcp;

class NetworkClient {
private:
    boost::asio::io_context io_context;
    tcp::socket socket;
    string serverhost;
    short serverport;
    bool connected;
    mutable mutex socketmutex;
public:
    NetworkClient(const string& host = "localhost", short port = 8080);
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
    json get_questions(int category_id);
    json check_answers(const string& user_login, int category_id, const json& answers);
    json get_statistics(const string& user_login);
    json get_resource_categories();
    json get_resources_by_category(int category_id);
    json get_all_resources(); 
    json get_resources_with_categories();
};
extern NetworkClient* g_network_client;
#endif
